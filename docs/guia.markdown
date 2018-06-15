# Guia do programador

## Diretórios
 - antena-arduino: contém o código Arduino
 - build: contém o executável
 - docs: especificações, relatórios
 - doxygen: documentação gerada automaticamente
 - spg4: biblioteca do DanRW com DateTime modificado
 - src: código-fonte
    - emmn-gui: código do projeto
    - emmn-tests: unit tests
    - includes: cabeçalhos das bibliotecas
    - libs: arquivos `.a` (bibliotecas estáticas)

## Estrutura do código

O ponto de entrada do programa está no arquivo main.cpp. Na função main, uma MainWindow é
instanciada e mostrada ao usuário. O construtor de MainWindow, por sua vez, instancia:

 - Control: responsável pela movimentação da antena
 - Network: responsável pela comunicação com a Internet para atualizar TLEs
 - Logger: utilidade para salvar arquivos com dados de posição da antena e timestamps

### Bibliotecas
 - SGP4: recebe TLEs e gera posições de azimute e elevação
 - SolTrack: rastreia o sol
 - CSerial: comunicação com porta serial

## Documentação

 - O projeto é documentado com Doxygen. Para gerar a documentação, rode o Doxywizard
     - usando o prompt de comando (eu uso o Cmder), `doxywizard.exe` ou `doxywizard.exe Doxyfile`
     - através do Menu Iniciar: Digite `Doxywizard` e localize o Doxyfile na pasta principal do projeto
 - Para gerar um pdf da documentação (útil para colocar na biblioteca do INPE), vá para a pasta
   doxygen/latex, execute make.bat ou rode `make` no Cmder.

## Estilo

Você é livre para usar o estilo que quiser, mas caso queira continuar usando a formatação atual,
siga estes padrões:

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

## Miscelânea
 - Ao mudar o diretório de build pelo Qt, é preciso copiar os arquivos dll do `ssh` (`libeay32.dll` e `ssleay32.dll`) para o diretório que contém o executável.

## O que o programa deve fazer e faz
 - Adicionar e remover satélites para rastreio
 - Definir ordem de prioridade de satélites para rastreio
 - Atualizar TLEs automaticamente todo dia
 - Enviar comandos de apontamento (azimute e elevação) para o Arduino
 - Controlar manualmente a antena (azimute e elevação arbitrários)
 - Gravar histórico de posicionamento da antena em arquivos `.csv`

## O que o programa deve fazer e (ainda) não faz
- Definição de parâmetros para cada satélite rastreado

## O que o programa poderia fazer e (ainda) não faz
Na documentação Doxygen é possível ver uma lista de tarefas extraídas dos comentários `@todo`. As melhorias por ordem de relevância são

- Atualizar lista de TLEs obtidas do SpaceTrack numa thread diferente. A maneira atual bloqueia a thread da interface
- Tirar userGeo da classe Tracker
- Instanciar Settings apenas na inicialização do programa
- Colocar velocidade máxima e aceleração da antena nas configurações
- A classe Control utiliza a biblioteca `CSerial`, mas o Qt possui funções para comunicação com porta serial no cabeçalho `QSerialPort`. A mudança para a biblioteca do Qt eliminaria uma dependência e possivelmente aumentaria a portabilidade
- Interação com o mouse no widget gráfico de próximas passagens
- Colocar localização geográfica nas configurações

## Classes

### Interface gráfica

#### MainWindow
Janela principal do programa. A classe faz a inicialização de todo o sistema e contém timers para leitura de estado da antena.

#### AddTrackerDialog
Caixa de diálogo para inserção de um satélite

#### JoystickWidget
Widget para controle da antena através de um joystick virtual.

#### ManualControlDialog


#### Control
Lógica de manipulação da antena. Envia e recebe comandos de hardware

#### Helpers
Contém funções auxiliares divididas em dois namespaces. Settings contém funções para carregar e salvar configurações. Helpers contém funções de conversões matemáticas, formatação de data e horários, leituras de arquivo etc.

#### Logger
Classe responsável pelo registro periódico de informações contendo posição da antena em azimute e elevação, velocidade, etc.

#### Network
#### NextPassesView
#### Serial
#### SettingsDialog
#### Serial
#### SolTrack
#### Tracker
#### TrackerListModel

## Configurações

### emmn-gui.pro
Atualizada automaticamente pelo Qt toda vez que uma classe é criada. Informa ao compilador quais são os arquivos utilizados pelo programa.

### emmn.pri
Arquivo de includes do Qt.
