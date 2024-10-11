
// function main ( ) 
//   const a = 4;
//   const b = "I'm a string";

//   print(a);
//   print(b);
// ..

void L528_main() {
    RuntimeObject* local1 = make_int(4);
    RuntimeObject* local2 = make_string("I'm a string");
    builtin_print(local1);
    builtin_print(local2);
}

int main(int argc, char **argv) {
    L528_main();
    return 0;
}