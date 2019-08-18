package org.lsst.ccs.daq.ims;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.nio.ByteBuffer;
import java.nio.channels.ByteChannel;
import java.time.Duration;

/**
 * A channel for reading or writing DAQ raw data
 *
 * @author tonyj
 */
class DAQSourceChannel implements ByteChannel {

    static ByteChannel open(Source source, Source.ChannelMode mode) throws DAQException {
        switch (mode) {
            case READ:
                if (source.getMetaData().getLength() == 0) {
                    throw new DAQException("Cannot read from source which is not fully written, did you mean to use ChennelMode.STREAM?");
                }
                return new ReadableDAQSourceChannel(source);
            case STREAM:
                return new StreamingDAQSourceChannel(source);
            case WRITE:
                return new WritableDAQSourceChannel(source);
            default:
                throw new UnsupportedOperationException("Unsupported mode " + mode);
        }
    }

    protected final Store store;
    protected long channel_;
    private final Source.ChannelMode mode;

    DAQSourceChannel(Source source, Source.ChannelMode mode) throws DAQException {
        final Image image = source.getImage();
        store = image.getStore();
        if (mode != Source.ChannelMode.STREAM) {
            // In the case of streaming we delay opening the channel until the first data is ready 
            channel_ = store.openSourceChannel(image.getMetaData().getId(), source.getLocation(), mode);
        }
        this.mode = mode;
    }

    @Override
    public boolean isOpen() {
        return channel_ != 0;
    }

    @Override
    public void close() throws IOException {
        if (channel_ != 0) {
            synchronized (store) {
                close(channel_, mode == Source.ChannelMode.WRITE);
            }
            channel_ = 0;
        }
    }

    void checkOpen(ByteBuffer dst) throws IOException {
        if (channel_ == 0) {
            throw new IOException("Channel not open for read");
        }
        if (!dst.isDirect()) {
            throw new IOException("Supplied ByteBuffer is not direct");
        }
    }

    @Override
    public int read(ByteBuffer dst) throws IOException {
        throw new IOException("Channel not open for read.");
    }

    @Override
    public int write(ByteBuffer src) throws IOException {
        throw new IOException("Channel not open for write.");
    }

    protected native int read(long source_, ByteBuffer dst, int position, long offset, int length);

    protected native int write(long source_, ByteBuffer src, int position, int remaining);

    private native void close(long source_, boolean write);

    static class ReadableDAQSourceChannel extends DAQSourceChannel {

        private final long length;
        private long offset = 0;

        ReadableDAQSourceChannel(Source source) throws DAQException {
            super(source, Source.ChannelMode.READ);
            length = source.getMetaData().getLength();
        }

        @Override
        public int read(ByteBuffer dst) throws IOException {
            checkOpen(dst);
            int l = (int) Math.min(dst.remaining(), length - offset);
            if (l == 0) {
                return -1;
            }
            int position = dst.position();
            int err;
            synchronized (store) {
                err = read(channel_, dst, position, offset, l);
            }
            if (err != 0) {
                DAQException daq = new DAQException("Error reading DAQ data", err);
                throw new IOException("DAQ IO exception", daq);
            }
            offset += l;
            dst.position(position + l);
            return l;
        }
    }

    static class StreamingDAQSourceChannel extends DAQSourceChannel implements StreamListener {

        private volatile long length = 0;
        private volatile boolean isComplete = false;
        private long offset = 0;
        private final Source source;
        private static final Duration READ_TIMEOUT = Duration.ofSeconds(10);

        @SuppressWarnings("LeakingThisInConstructor")
        StreamingDAQSourceChannel(Source source) throws DAQException {
            super(source, Source.ChannelMode.STREAM);
            this.source = source;
            source.addStreamListener(this);
        }

        @Override
        public int read(ByteBuffer dst) throws IOException {
            try {
                int l = waitForData(dst);
                if (l < 0) {
                    return -1;
                }
                if (channel_ == 0) {
                   final Image image = source.getImage();
                   channel_ = store.openSourceChannel(image.getMetaData().getId(), source.getLocation(), Source.ChannelMode.STREAM);
                }
                checkOpen(dst);
                int position = dst.position();
                int err;
                // TODO: Check if synchronizing on store is really necessary. Mike says not.
                synchronized (store) {
                    System.out.printf("Read %d %d %d %d\n", channel_, position, offset, l);
                    err = read(channel_, dst, position, offset, l);
                }
                if (err != 0) {
                    throw new DAQException("Error reading DAQ data", err);
                }
                offset += l;
                dst.position(position + l);
                return l;
            } catch (DAQException x) {
                throw new IOException("DAQ IO exception", x);
            }
        }

        private synchronized int waitForData(ByteBuffer dst) throws IOException {
            int l = (int) Math.min(dst.remaining(), length - offset);
            long finishBy = System.currentTimeMillis() + READ_TIMEOUT.toMillis();
            while (l == 0) {
                try {
                    if (isComplete) {
                        return -1;
                    } else {
                        long remainingTime = finishBy - System.currentTimeMillis();
                        if (remainingTime <= 0) {
                            throw new IOException("Streaming read timed out");
                        }
                        wait(remainingTime);
                    }
                    l = (int) Math.min(dst.remaining(), length - offset);
                } catch (InterruptedException x) {
                    throw new InterruptedIOException();
                }
            }
            return l;
        }

        @Override
        public synchronized void streamLength(long length) {
            this.length = length;
            this.notifyAll();
        }

        @Override
        public synchronized void imageComplete(long length) {
            this.length = length;
            this.isComplete = true;
            this.notifyAll();
        }

        @Override
        public void close() throws IOException {
            source.removeStreamListener(this);
            super.close();
        }

    }

    private static class WritableDAQSourceChannel extends DAQSourceChannel {

        public WritableDAQSourceChannel(Source source) throws DAQException {
            super(source, Source.ChannelMode.WRITE);
        }

        @Override
        public int write(ByteBuffer src) throws IOException {
            checkOpen(src);
            int remaining = src.remaining();
//            if ((remaining % (16*3)) != 0) {
//                throw new IOException("ByteBuffer size must be a multiple of 48");
//            }
            int position = src.position();
            int err;
            synchronized (store) {
                err = write(channel_, src, position, remaining);
            }
            if (err != 0) {
                DAQException daq = new DAQException("Error reading DAQ data", err);
                throw new IOException("DAQ IO exception", daq);
            }
            src.position(position + remaining);
            return remaining - position;
        }
    }

}
