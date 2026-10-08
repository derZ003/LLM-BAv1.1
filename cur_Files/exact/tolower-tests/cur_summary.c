#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
int tolower(int c) {
    symbolic sc = (symbolic)(unsigned int)c;
    cnstr_t is_upper = _AND_(_GE_(sc, 'A'), _LE_(sc, 'Z'));
    symbolic lowered = (symbolic)((unsigned int)sc + ('a' - 'A'));
    
    if (is_certain(is_upper)) {
        return (int)lowered;
    } else if (is_certain(_NOT_(is_upper))) {
        return (int)sc;
    } else {
        push_pc();
        assume(is_upper);
        symbolic a = lowered;
        pop_pc();
        push_pc();
        assume(_NOT_(is_upper));
        symbolic b = sc;
        pop_pc();
        return (int)_ITE_VAR_(is_upper, a, b);
    }
}
