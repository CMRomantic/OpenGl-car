package com.aonions.opengl.jni;

/**
 * Created by  on 2024/3/20.
 */
public class IoHandle {
    public int devOpen;
    public int bResendFlag;
    public byte usCheckSum;
    public int uCmdIndex;
    public byte[] buffer = new byte[65];
    public DevIo mDevIo;

    public static class DevIo {
        public void init() {
        }

        public int open() {
            return 0;
        }

        public void close() {
        }

        public int write(int dwMilliseconds, byte[] pcBuffer) {
            return 0;
        }

        public int read(int dwMilliseconds, byte[] pcBuffer) {
            return 0;
        }
    }
}
