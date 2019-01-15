package net.callrec.library.recorder;

import net.callrec.library.fix.LibLoader;

public class AudioRecordNative {
    static {
        LibLoader.loadLib();
        nativeInit();
    }

    public AudioRecordNative(int audiosource, int samplerate, int audioformat,
                             int channels, int size) {
        nativeCreate(audiosource, samplerate, audioformat, channels, size);
    }

    public int start() {
        return nativeStart();
    }

    public int stop() {
        return nativeStop();
    }

    public byte[] read(byte[] buffer, int buffersize) {
        return nativeRead(buffer, buffersize);
    }

    public boolean destroy() {
        return nativeDestroy();
    }

    public static native boolean nativeInit();

    public static native boolean nativeDestroy();

    public static native int nativeStart();

    public static native boolean nativeCreate(int audiosource, int samplerate,
                                              int audioformat, int channels, int size);

    public static native int nativeStop();

    public static native byte[] nativeRead(byte[] buffer, int bufferSize);

}
