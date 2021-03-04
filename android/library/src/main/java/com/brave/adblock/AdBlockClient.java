package com.brave.adblock;

public class AdBlockClient {
    private long nativeThis;

    public enum FilterOption {
        UNKNOWN(0),
        SCRIPT(1),
        CSS(4),
        OBJECT(10),
        IMAGE(2);

        public final int value;

        private FilterOption(int value) {
            this.value = value;
        }
    }

    public AdBlockClient() {
        init();
    }

    public native void init();
    public native void deinit();
    public native boolean parse(String input);
    public native boolean parseFile(String fileName);
    public native boolean serialize(String fileName);
    public native boolean deserialize(String fileName);
    public native boolean matches(String urlToCheck, int filterOption, String contextDomain);

    public boolean matches(String urlToCheck, FilterOption filterOption, String contextDomain) {
        return matches(urlToCheck, filterOption.value, contextDomain);
    }

    @Override
    protected void finalize() throws Throwable {
        deinit();
        super.finalize();
    }

    static {
        System.loadLibrary("ad-block");
    }
}
