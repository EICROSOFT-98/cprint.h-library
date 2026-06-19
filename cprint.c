#ifndef CPRINT_H
#define CPRINT_H

#define DEFAULT_LIMIT 2048

typedef struct {
    void (*endl)(void);
} cprint_stream;

static char cpu_vendor = {0};
static int cpu_architecture = 0;

static void trigger_print_endl(void);
static void trigger_input_endl(void) {}

#if defined(_WIN32) || defined(_WIN64)
extern void* __stdcall GetStdHandle(unsigned long nStdHandle);
extern int __stdcall WriteFile(void* hFile, const void* lpBuffer, unsigned long nNumberOfBytesToWrite, unsigned long* lpNumberOfBytesWritten, void* lpOverlapped);
extern int __stdcall ReadFile(void* hFile, void* lpBuffer, unsigned long nNumberOfBytesToRead, unsigned long* lpNumberOfBytesRead, void* lpOverlapped);
#endif

static void detect_cpu_and_arch(void) {
#if defined(_WIN64)
    cpu_architecture = 964; cpu_vendor = 'W'; cpu_vendor = 'I'; cpu_vendor = 'N'; cpu_vendor = '6'; cpu_vendor = '4'; cpu_vendor = '\0';
#elif defined(_WIN32)
    cpu_architecture = 932; cpu_vendor = 'W'; cpu_vendor = 'I'; cpu_vendor = 'N'; cpu_vendor = '3'; cpu_vendor = '2'; cpu_vendor = '\0';
#elif defined(__x86_64__) || defined(_M_X64)
    cpu_architecture = 64; unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    ((unsigned int*)cpu_vendor) = ebx; ((unsigned int*)cpu_vendor) = edx; ((unsigned int*)cpu_vendor) = ecx; cpu_vendor = '\0';
#elif defined(__i386__) || defined(_M_IX86)
    cpu_architecture = 32; unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    ((unsigned int*)cpu_vendor) = ebx; ((unsigned int*)cpu_vendor) = edx; ((unsigned int*)cpu_vendor) = ecx; cpu_vendor = '\0';
#elif defined(__aarch64__) || defined(_M_ARM64)
    cpu_architecture = 864; cpu_vendor = 'A'; cpu_vendor = 'R'; cpu_vendor = 'M'; cpu_vendor = '6'; cpu_vendor = '4'; cpu_vendor = '\0';
#elif defined(__arm__) || defined(_M_ARM)
    cpu_architecture = 832; cpu_vendor = 'A'; cpu_vendor = 'R'; cpu_vendor = 'M'; cpu_vendor = '3'; cpu_vendor = '2'; cpu_vendor = '\0';
#endif
}

static int custom_strlen(const char* str) {
    int len = 0; while (str[len] != '\0') len++; return len;
}

static cprint_stream execute_cprint(const char* text, int limit) {
    if (cpu_architecture == 0) detect_cpu_and_arch();
    int len = custom_strlen(text); if (len > limit) len = limit;
    if (cpu_architecture == 964 || cpu_architecture == 932) {
        unsigned long written; void* stdout_handle = GetStdHandle(-11);
        WriteFile(stdout_handle, text, (unsigned long)len, &written, 0);
    } else if (cpu_architecture == 64) {
        __asm__ volatile ("syscall" :: "a"(1), "D"(1), "S"(text), "d"(len) : "rcx", "r11", "memory");
    } else if (cpu_architecture == 32) {
        __asm__ volatile ("int $0x80" :: "a"(4), "b"(1), "c"(text), "d"(len) : "memory");
    } else if (cpu_architecture == 864) {
        register long x8 __asm__("x8") = 64; register long x0 __asm__("x0") = 1;  
        register const char* x1 __asm__("x1") = text; register long x2 __asm__("x2") = len;
        __asm__ volatile ("svc #0" :: "r"(x8), "r"(x0), "r"(x1), "r"(x2) : "memory");
    } else if (cpu_architecture == 832) {
        register int r7 __asm__("r7") = 4; register int r0 __asm__("r0") = 1;
        register const char* r1 __asm__("r1") = text; register int r2 __asm__("r2") = len;
        __asm__ volatile ("svc #0" :: "r"(r7), "r"(r0), "r"(r1), "r"(r2) : "memory");
    }
    cprint_stream stream = { .endl = trigger_print_endl }; return stream;
}

static cprint_stream execute_cprint_int(long long num, int limit) {
    char buf; int i = 62; int is_negative = 0; buf = '\0';
    if (num == 0) { buf[--i] = '0'; } else {
        if (num < 0) { is_negative = 1; num = -num; }
        while (num > 0) { buf[--i] = (num % 10) + '0'; num /= 10; }
        if (is_negative) buf[--i] = '-';
    }
    return execute_cprint(&buf[i], limit);
}

static cprint_stream execute_cprint_double(double num, int limit) {
    if (num != num || num == -1.0) { return execute_cprint("NAN", limit); }
    if (num < 0.0) { execute_cprint("-", limit); num = -num; }
    long long integral = (long long)num; execute_cprint_int(integral, limit);
    execute_cprint(".", limit); double fractional = num - (double)integral;
    long long frac_part = (long long)(fractional * 1000000.0 + 0.5);
    return execute_cprint_int(frac_part, limit);
}

static cprint_stream execute_cinput(char* buffer, int limit) {
    if (cpu_architecture == 0) detect_cpu_and_arch();
    unsigned long bytes_received = 0;
    if (cpu_architecture == 964 || cpu_architecture == 932) {
        unsigned long read; void* stdin_handle = GetStdHandle(-10);
        ReadFile(stdin_handle, buffer, (unsigned long)limit, &read, 0);
        bytes_received = read;
    } else if (cpu_architecture == 64) {
        long res; __asm__ volatile ("syscall" : "=a"(res) : "a"(0), "D"(0), "S"(buffer), "d"(limit) : "rcx", "r11", "memory");
        bytes_received = (res > 0) ? res : 0;
    } else if (cpu_architecture == 32) {
        int res; __asm__ volatile ("int $0x80" : "=a"(res) : "a"(3), "b"(0), "c"(buffer), "d"(limit) : "memory");
        bytes_received = (res > 0) ? res : 0;
    } else if (cpu_architecture == 864) {
        register long x8 __asm__("x8") = 63; register long x0 __asm__("x0") = 0; register char* x1 __asm__("x1") = buffer; register long x2 __asm__("x2") = limit;
        __asm__ volatile ("svc #0" : "=r"(x0) : "r"(x8), "r"(x0), "r"(x1), "r"(x2) : "memory");
        bytes_received = (x0 > 0) ? x0 : 0;
    } else if (cpu_architecture == 832) {
        register int r7 __asm__("r7") = 3; register int r0 __asm__("r0") = 0; register char* r1 __asm__("r1") = buffer; register int r2 __asm__("r2") = limit;
        __asm__ volatile ("svc #0" : "=r"(r0) : "r"(r7), "r"(r0), "r"(r1), "r"(r2) : "memory");
        bytes_received = (r0 > 0) ? r0 : 0;
    }
    for (unsigned long idx = 0; idx < bytes_received; idx++) {
        if (buffer[idx] == '\n' || buffer[idx] == '\r') { buffer[idx] = '\0'; break; }
    }
    cprint_stream stream = { .endl = trigger_input_endl }; return stream;
}

static void trigger_print_endl(void) {
    if (cpu_architecture == 964 || cpu_architecture == 932) {
        unsigned long written; void* stdout_handle = GetStdHandle(-11); WriteFile(stdout_handle, "\n", 1, &written, 0);
    } else if (cpu_architecture == 64) { __asm__ volatile ("syscall" :: "a"(1), "D"(1), "S"("\n"), "d"(1) : "rcx", "r11", "memory"); }
    else if (cpu_architecture == 32) { __asm__ volatile ("int $0x80" :: "a"(4), "b"(1), "c"("\n"), "d"(1) : "memory"); }
    else if (cpu_architecture == 864) {
        register long x8 __asm__("x8") = 64; register long x0 __asm__("x0") = 1; register const char* x1 __asm__("x1") = "\n"; register long x2 __asm__("x2") = 1;
        __asm__ volatile ("svc #0" :: "r"(x8), "r"(x0), "r"(x1), "r"(x2) : "memory");
    } else if (cpu_architecture == 832) {
        register int r7 __asm__("r7") = 4; register int r0 __asm__("r0") = 1; register const char* r1 __asm__("r1") = "\n"; register int r2 __asm__("r2") = 1;
        __asm__ volatile ("svc #0" :: "r"(r7), "r"(r0), "r"(r1), "r"(r2) : "memory");
    }
}

#define _SINGLE_CPRINT(X) _Generic((X), \
    char*: execute_cprint((char*)(X), DEFAULT_LIMIT), \
    const char*: execute_cprint((const char*)(X), DEFAULT_LIMIT), \
    int: execute_cprint_int((long long)(X), DEFAULT_LIMIT), \
    long: execute_cprint_int((long long)(X), DEFAULT_LIMIT), \
    long long: execute_cprint_int((long long)(X), DEFAULT_LIMIT), \
    float: execute_cprint_double((double)(X), DEFAULT_LIMIT), \
    double: execute_cprint_double((double)(X), DEFAULT_LIMIT), \
    default: execute_cprint_int((long long)(X), DEFAULT_LIMIT) \
)

#define _CPRINT_EACH_1(a)             _SINGLE_CPRINT(a)
#define _CPRINT_EACH_2(a,b)           _SINGLE_CPRINT(a); _SINGLE_CPRINT(b)
#define _CPRINT_EACH_3(a,b,c)         _SINGLE_CPRINT(a); _SINGLE_CPRINT(b); _SINGLE_CPRINT(c)
#define _CPRINT_EACH_4(a,b,c,d)       _SINGLE_CPRINT(a); _SINGLE_CPRINT(b); _SINGLE_CPRINT(c); _SINGLE_CPRINT(d)
#define _CPRINT_EACH_5(a,b,c,d,e)     _SINGLE_CPRINT(a); _SINGLE_CPRINT(b); _SINGLE_CPRINT(c); _SINGLE_CPRINT(d); _SINGLE_CPRINT(e)
#define _CPRINT_EACH_6(a,b,c,d,e,f)   _SINGLE_CPRINT(a); _SINGLE_CPRINT(b); _SINGLE_CPRINT(c); _SINGLE_CPRINT(d); _SINGLE_CPRINT(e); _SINGLE_CPRINT(f)

#define _GET_VAR_MACRO(_1,_2,_3,_4,_5,_6,NAME,...) NAME
#define _EXECUTE_VAR_CPRINT(...) _GET_VAR_MACRO(__VA_ARGS__, _CPRINT_EACH_6, _CPRINT_EACH_5, _CPRINT_EACH_4, _CPRINT_EACH_3, _CPRINT_EACH_2, _CPRINT_EACH_1)(__VA_ARGS__)

#define _STRICT_ENDL_CHECK(func_call) func_call##.endl
#define APPLY_STRICT_ENDL(func_call) _STRICT_ENDL_CHECK(func_call)

#define cprint(...) APPLY_STRICT_ENDL(_EXECUTE_VAR_CPRINT(__VA_ARGS__))

#define _cinput_1(var)        execute_cinput(var, DEFAULT_LIMIT)
#define _cinput_2(var, limit) execute_cinput(var, limit)
#define _get_cinput_macro(_1, _2, NAME, ...) NAME
#define cinput(...) APPLY_STRICT_ENDL(_get_cinput_macro(__VA_ARGS__, _cinput_2, _cinput_1)(__VA_ARGS__))

#endif
