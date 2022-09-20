import java.util.ArrayList;
import java.util.Random;
import java.util.function.Consumer;

public class ReaderWriterSample {

    //------------------------------------------------------------------------
    // Avoid usage of checked InterruptedException
    // in  favor of unchecked RuntimeInterruptedException
    //------------------------------------------------------------------------
    public static class RuntimeInterruptedException extends RuntimeException  {
        private static final long serialVersionUID = 8454424941904296017L;
        public RuntimeInterruptedException() {
        }
        public RuntimeInterruptedException(Exception e) {
            super(e);
        }
        public RuntimeInterruptedException of(Exception e) {
            return new RuntimeInterruptedException(e);
        }
    }
    //------------------------------------------------------------------------
    // wait/sleep alternatives throwing unchecked RuntimeInterruptedException
    //------------------------------------------------------------------------
    public static void waitFor(Object o) {
        waitFor(o, -1);
    }
    public static void waitFor(Object o, long millis) {
        try {
            if( millis < 0 ) {
                o.wait();
            } else {
                o.wait(millis);
            }
        } catch( InterruptedException e ) {
            throw new RuntimeInterruptedException(e);
        }
    }
    public static void sleep(long millis) {
        sleep(millis, null);
    }
    public static void sleep(long millis, Consumer<Exception> onInterrupted) {
        try {
            if( millis < 0 ) {
                millis = 0;
            }
            Thread.sleep(millis);
        } catch( InterruptedException e ) {
            if( onInterrupted == null ) {
                throw new RuntimeInterruptedException(e);
            } else {
                onInterrupted.accept(e);
            }
        } catch( RuntimeInterruptedException e ) {
            if( onInterrupted == null ) {
                throw e;
            } else {
                onInterrupted.accept(e);
            }
        }
    }

    //------------------------------------------------------------------------
    // Thread management utilities
    //------------------------------------------------------------------------
    public static Thread getThreadByName(String threadName) {
        for (Thread t : Thread.getAllStackTraces().keySet()) {
            if (t.getName().equals(threadName)) return t;
        }
        return null;
    }
    public static Thread runNamedDaemonThread(String threadName, Runnable r) {
        Thread ret = new Thread() {
            @Override
            public void run() {
                r.run();
            }
        };
        ret.setName(threadName);
        ret.setDaemon(true);
        ret.start();
        return ret;
    }
    public static Thread runNamedDaemonThreadIfNoneYet(String threadName, Runnable r) {
        Thread ret=getThreadByName(threadName);
        if( ret==null ) {
            ret = runNamedDaemonThread(threadName, r);
        }
        return ret;
    }

    //--------------------------------------------------------------------
    // A self-builder for Runnable instances
    //--------------------------------------------------------------------
    public static class ForeverRunnableWrapper implements Runnable {
        Runnable r;
        long periodMs;
        long pauseAtStartMs;
        long lastRun;
        Consumer<RuntimeException> onException;
        Consumer<Exception> onInterrupted;
        private ForeverRunnableWrapper() {}
        public static ForeverRunnableWrapper of(Runnable r) {
            ForeverRunnableWrapper ret = new ForeverRunnableWrapper();
            ret.r = r;
            return ret;
        }
        public ForeverRunnableWrapper copy() {
            ForeverRunnableWrapper ret = new ForeverRunnableWrapper();
            ret.r = r;
            ret.periodMs = periodMs;
            ret.pauseAtStartMs = pauseAtStartMs;
            ret.lastRun = lastRun;
            ret.onException = onException;
            ret.onInterrupted = onInterrupted;
            return ret;
        }
        public ForeverRunnableWrapper periodMs(long periodMs) {
            this.periodMs = periodMs;
            return this;
        }
        public ForeverRunnableWrapper pauseAtStartMs(long pauseAtStartMs) {
            this.pauseAtStartMs = pauseAtStartMs;
            return this;
        }
        public ForeverRunnableWrapper onRuntimeException(Consumer<RuntimeException> onException) {
            this.onException = onException;
            return this;
        }
        public ForeverRunnableWrapper onInterrupted(Consumer<Exception> onInterrupted) {
            this.onInterrupted = onInterrupted;
            return this;
        }
        public long getLastRunMillis() {
            return lastRun;
        }
        @Override
        public void run() {
            sleep(pauseAtStartMs,onInterrupted);
            while( true ) {
                long start = System.currentTimeMillis();
                runTask();
                long end = System.currentTimeMillis();
                long spentInProcessing = start - end;
                long remains = periodMs - spentInProcessing;
                sleep(remains,onInterrupted);
            }
        }
        public void runTask() {
            try {
                lastRun = System.currentTimeMillis();
                r.run();
            } catch( RuntimeException e ) {
                if( onException == null ) {
                    throw e;
                } else {
                    onException.accept(e);
                }
            }
        }
    }

    public static ForeverRunnableWrapper foreverRunnable( Runnable repeatableAction ) {
        return ForeverRunnableWrapper.of(repeatableAction);
    }

    //--------------------------------------------------------------------
    // Defines a lock that supports single writers and multiple readers.
    //--------------------------------------------------------------------
    public static class ReadWriteLock {
        private int numReaders = 0;
        private boolean isWriterActive = false;
        private int numWritersWaiting = 0;
        public class ReadLock implements AutoCloseable {
            public ReadLock() {
                lockRead();
            }
            @Override
            public void close() throws RuntimeException {
                unlockRead();
            }
        }
        public ReadLock readLock() {
            return new ReadLock();
        }
        public class WriteLock implements AutoCloseable {
            public WriteLock() {
                lockWrite();
            }
            @Override
            public void close() throws RuntimeException {
                unlockWrite();
            }
        }
        public WriteLock writeLock() {
            return new WriteLock();
        }
        private synchronized void lockRead() {
            while (isWriterActive || numWritersWaiting > 0) {
                waitFor(this);
            }
            numReaders++;
        }
        private synchronized void unlockRead() {
            numReaders--;
            notifyAll();
        }
        private synchronized void lockWrite() {
            numWritersWaiting++;
            while (numReaders > 0 || isWriterActive) {
                waitFor(this);
            }
            numWritersWaiting--;
            isWriterActive=true;
        }
        private synchronized void unlockWrite() {
            isWriterActive=false;
            notifyAll();
        }
    }

    //-----------------------------------------------------------------------
    // Let's test ReaderWriter functionality
    //-----------------------------------------------------------------------
    public static void main(String[] args) {
        long leftBalance[] = {50};
        long rightBalance[] = {50};
        ReadWriteLock bufferMonitor = new ReadWriteLock();

        ArrayList<String> messages = new ArrayList<String>();
        Object messagesMonitor = new String();

        Random rand = new Random();
        runNamedDaemonThread("Logger",
            foreverRunnable(()->{
                ArrayList<String> myMessages = new ArrayList<String>();
                synchronized(messagesMonitor) {
                    myMessages.addAll(messages);
                    messages.clear();
                }
                for (String m : myMessages) {
                    System.out.println(m);
                }
            })
            .periodMs(10)
            .onRuntimeException((e)->{e.printStackTrace(); throw e;})
            .onInterrupted((e)->{e.printStackTrace(); throw new RuntimeInterruptedException(e);})
        );
        ForeverRunnableWrapper writer = foreverRunnable(()->{
                long change = (rand.nextLong()%20)-(rand.nextLong()%20);
                long l,r;
                try( ReadWriteLock.WriteLock lock = bufferMonitor.writeLock() ) {
                    l=(leftBalance[0]+=change);
                    r=(rightBalance[0]-=change);
                }
                synchronized(messagesMonitor) {
                    messages.add(Thread.currentThread().getName()
                            + ": total="+(l+r)+" left="+l+" right="+r);
                }
                sleep(10);
            })
            .periodMs(10)
            .onRuntimeException((e)->{e.printStackTrace(); throw e;})
            .onInterrupted((e)->{e.printStackTrace(); throw new RuntimeInterruptedException(e);})
        ;

        runNamedDaemonThread("Writer N1", writer.pauseAtStartMs(1).periodMs(15) );
        runNamedDaemonThread("Writer N2", writer.copy().pauseAtStartMs(20).periodMs(33) );
        Runnable reader =
            foreverRunnable(()->{
                long l,r;
                try( ReadWriteLock.ReadLock lock = bufferMonitor.readLock() ) {
                    l=leftBalance[0];
                    r=rightBalance[0];
                }
                synchronized(messagesMonitor) {
                    messages.add(Thread.currentThread().getName()
                            + ": total="+(l+r)+" left="+l+" right="+r);
                }
                sleep(5);
            })
            .periodMs(1)
            .pauseAtStartMs(5)
            .onRuntimeException((e)->{e.printStackTrace(); throw e;})
            .onInterrupted((e)->{e.printStackTrace(); throw new RuntimeInterruptedException(e);})
        ;
        for( int i=0; i<9; ++i ) {
            runNamedDaemonThread("Reader N"+i,reader);
        }
        sleep(500);
    }
}
