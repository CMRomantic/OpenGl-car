package com.aonions.opengl.jni;

/**
 * Created by  on 2024/3/18.
 */
public class ISPJni {

   static {
      System.loadLibrary("aonions");
   }


   public static native short checksum(byte[] buffer, int len);

   public static native int open(IoHandle handle);

   public static native void close(IoHandle handle);

   public static native void updateConfig(IoHandle handle, int[] config, int[] response);

   public static native void readConfig(IoHandle handle, int[] config);

   public static native void syncPackNo(IoHandle handle);

   public static native int connect(IoHandle handle, int dwMilliseconds);

   public static native int resend(IoHandle handle);
}
