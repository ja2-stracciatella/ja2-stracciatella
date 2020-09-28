package io.github.ja2stracciatella

// This class is used from c++ code to communicate native exceptions using std::set_terminate
object NativeExceptionContainer {
    private var exception: String? = null

    @Synchronized
    fun getException(): String? {
        return this.exception
    }

    @Synchronized
    fun setException(exception: String) {
        this.exception = exception
    }

    @Synchronized
    fun resetException() {
        this.exception = null
    }
}