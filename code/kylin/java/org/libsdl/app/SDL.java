package org.libsdl.app;

import android.content.Context;

import java.lang.reflect.*;

/**
    SDL library initialization
*/
public class SDL {

    // This function should be called first and sets up the native code
    // so it can call into the Java classes
    public static void setupJNI() {
        SDLActivity.nativeSetupJNI();
        SDLAudioManager.nativeSetupJNI();
    }

    // This function should be called each time the activity is started
    public static void initialize() {
        setContext(null);

        SDLActivity.initialize();
        SDLAudioManager.initialize();
    }

    // This function stores the current activity (SDL or not)
    public static void setContext(Context context) {
        mContext = context;
    }

    public static Context getContext() {
        return mContext;
    }

    public static void loadLibrary(String libraryName) throws UnsatisfiedLinkError, SecurityException, NullPointerException {

        if (libraryName == null) {
            throw new NullPointerException("No library name provided.");
        }

        try {
            System.loadLibrary(libraryName);
        }
        catch (final UnsatisfiedLinkError ule) {
            throw ule;
        }
        catch (final SecurityException se) {
            throw se;
        }
    }

    protected static Context mContext;
}
