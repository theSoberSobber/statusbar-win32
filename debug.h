#ifdef DEBUG
void dbg(const char *errstr, ...) {
    va_list ap;
    va_start(ap, errstr);
    vfprintf(stderr, errstr, ap);
    fflush(stderr);
    va_end(ap);
}
#else
#define dbg
#endif