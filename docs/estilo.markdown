- 4 (quatro) espaços para indentação
- Espaço entre `if`, `while`, `for` etc e `(`, `{` na mesma linha: `if (ele > 180) {`
- Construtores com member initializer lists [1] deixam um { numa nova linha
- switches são formatados (assim)[2]:
```
switch (var) {
case 1:
    // alguma coisa
case 3:
    // outra coisa
default:
    // mais uma coisa
}
```

- O projeto é documentado com Doxygen. Para gerar a documentação, rode o Doxywizard
    - Menu Iniciar -> Digite `Doxywizard` e localize o Doxyfile na pasta principal
    - Usando o prompt de comando (eu uso o Cmder), `doxywizard.exe` ou `doxywizard.exe Doxyfile`

[1]: http://en.cppreference.com/w/cpp/language/initializer_list
[2]: http://www.oracle.com/technetwork/java/javase/documentation/codeconventions-142311.html#468
