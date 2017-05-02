# Frida Detection Examples

Some random ideas for detecting Frida instrumentation from within a process:

- Scan all local TCP ports, sending a D-Bus message to each port to identify fridaserver.
- Scan text sections for a string found inside <code>frida-gadget*.so</code> / <code>frida-agent*.so</code>. File operations are implemented in ASM so prevent easy bypassing with libc function hooks.

The techniques are discussed in this [blog post]. They were developed for the reverse engineering and (anti-)anti-tampering chapters of the [OWASP Mobile Security Testing Guide](https://github.com/OWASP/owasp-mstg). Note that this is only proof-of-concept code - it doesn't guarantee any kind of protection.
