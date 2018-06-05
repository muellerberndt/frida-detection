# Frida Detection Examples

Some random ideas for detecting Frida instrumentation from within a process:

- Scan all local TCP ports, sending a D-Bus message to each port to identify fridaserver.
- Scan text sections for a string found inside <code>frida-gadget*.so</code> / <code>frida-agent*.so</code>. File operations are implemented in ASM so prevent easy bypassing with libc function hooks.

These examples were developed to accompany a [blog post](https://trello.com/c/IAEd2pI0/77-api-terms-of-use). **Note that copy/pasting this into your own code will not guarantee any meaningful protection**.
