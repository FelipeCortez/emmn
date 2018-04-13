# Guia do programador

## Estrutura

O ponto de entrada do programa está no arquivo main.cpp. Na função main, uma MainWindow é
instanciada e mostrada ao usuário. O construtor de MainWindow, por sua vez, instancia:

- Control, responsável pela movimentação da Antena
- Network, responsável pela comunicação com a Internet para atualizar TLEs
- Logger, utilidade para salvar arquivos com dados de posição da antena e timestamps

## Documentação

- O projeto é documentado com Doxygen. Para gerar a documentação, rode o Doxywizard
    - usando o prompt de comando (eu uso o Cmder), `doxywizard.exe` ou `doxywizard.exe Doxyfile`
    - através do Menu Iniciar: Digite `Doxywizard` e localize o Doxyfile na pasta principal do projeto
- Para gerar um pdf da documentação (útil para colocar na biblioteca do INPE), vá para a pasta
  doxygen/latex e rode make.bat.

## Estilo

Você é livre para usar o estilo que quiser, mas caso queira continuar usando a formatação atual,
eu sigo estes padrões:

- Quatro espaços para indentação
- Espaço entre `if`, `while`, `for` etc e `(`
- `{` na mesma linha: `if (ele > 180) {`, exceto em construtores com [member initializer lists](http://en.cppreference.com/w/cpp/language/initializer_list)
- switches são formatados de acordo com a [convenção do Java](http://www.oracle.com/technetwork/java/javase/documentation/codeconventions-142311.html#468):
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
- Código em inglês, comentários em português
- Mensagens de commit do Git em português
