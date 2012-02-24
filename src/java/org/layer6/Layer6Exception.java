package org.layer6;

public class Layer6Exception extends Exception {
    int errorCode;

    public Layer6Exception(int code) {
        super(Layer6.getErrorMsg(code));
        errorCode = code;
    }
    
    public Layer6Exception(int code, String debug) {
        super(Layer6.getErrorMsg(code) + ":"+ debug);
        errorCode = code;
    }

    public Layer6Exception(int code, Exception ex) {
        super(ex);
        errorCode = code;
    }

    public int getErrorCode() {
        return errorCode;
    }
}