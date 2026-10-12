#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
int tolower(int c) {
    symbolic v = (symbolic)(unsigned long)c;
    cnstr_t is_upper = _AND_(_GE_(v, 'A'), _LE_(v, 'Z'));
    symbolic lower = (symbolic)((unsigned long)v + 32);

    if (is_certain(is_upper)) {
        return (int)(unsigned long)lower;
    } else if (is_certain(_NOT_(is_upper))) {
        return (int)(unsigned long)v;
    } else {
        push_pc();
        assume(is_upper);
        symbolic a = lower;
        pop_pc();
        push_pc();
        assume(_NOT_(is_upper));
        symbolic b = v;
        pop_pc();
        return (int)(unsigned long)_ITE_VAR_(is_upper, a, b);
    }
}
