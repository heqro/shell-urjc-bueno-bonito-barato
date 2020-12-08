<TeXmacs|1.99.14>

<style|<tuple|generic|spanish>>

<\body>
  <\hide-preamble>
    \;

    <assign|chapter-post-sep|<macro|<sectional-post-sep>>>
  </hide-preamble>

  <doc-data|<doc-title|Práctica 1 - Minishell>|<doc-author|<author-data|<author-name|Héctor
  Rodrigo Iglesias Goldaracena>>>|<doc-author|<author-data|<author-name|Y>>>|<doc-author|<author-data|<author-name|Juan
  Montes Cano>>>>

  <section|Índice de contenidos>

  <section|Descripción del código>

  <subsection|Funcionalidad implementada>

  El programa es capaz de cumplir los objetivos que se proponen en el
  enunciado de la práctica. En efecto, es capaz de:

  <\itemize>
    <item>reconocer y ejecutar tanto en <em|foreground> como en
    <em|background> líneas con un mandato con sus respectivos argumentos,

    <item>reconocer y ejecutar tanto en <em|foreground> como en
    <em|background> líneas con dos o más mandatos con sus respectivos
    argumentos, enlazados por medio de \S<math|<around*|\|||\<nobracket\>>>\T,

    <item>reconocer y aplicar redirección de entrada estándar desde archivo,
    y redirección de salida estándar y de salida de error a un archivo,

    <item>ejecutar los mandatos internos <verbatim|cd>, <verbatim|fg> y
    <verbatim|jobs>,

    <item>y evita que tanto los comandos en <em|background>, así como el
    Minishell, finalicen al enviar por teclado las señales <verbatim|SIGINT>
    y <verbatim|SIGQUIT>, mientras que permite que los procesos lanzados en
    <em|foreground> respondan ante ambas señales.
  </itemize>

  <subsection|Pseudocódigo y planteamiento del programa>

  <subsubsection|Planteamiento del programa>

  En primer lugar, el programa consta de un proceso principal,
  <verbatim|PShell>, que cuenta con las siguientes características:

  <\itemize>
    <item>nunca muere, a no ser que se lance <verbatim|EOF>
    (<verbatim|Ctrl+D>) en el caso de que nos encontremos en el
    <verbatim|fgets> que se encuentra a la cabeza del bucle principal,

    <item>nunca ejecuta ninguna instrucción, salvo aquellas implementadas por
    nuestro propio código (<verbatim|cd>, <verbatim|fg>, <verbatim|jobs>),

    <item>y cuenta con un manejador para <verbatim|SIGCHLD>.
  </itemize>

  Como consecuecuencia de estas dos propiedades, <verbatim|PShell> puede
  gestionar una lista de procesos en <em|background>, lo que nos permite
  marcarlos como acabados o en ejecución en función de en qué estado se
  encuentren.

  En caso de que se pida un mandato distinto a los tres de los que ofrecemos
  la implementación, se crea un hijo, <verbatim|PMandato> (al que
  <verbatim|PShell> esperará, si el mandato no se ejecuta en
  <em|background>), que será el encargado de supervisar la ejecución del
  mandato que se haya pedido, independientemente de que se esté ejecutando o
  no en <em|background>. Para ello, aplicará las redirecciones oportunas, que
  irán heredándose a cada uno de los procesos hijo que vaya originando.

  <verbatim|PMandato> creará, entonces, tantos hijos como mandatos haya en la
  línea cuya ejecución tenga que supervisar (de uno en uno, solo existirá un
  hijo en un instante de tiempo dado).

  Si <verbatim|PMandato> recibe una línea que se tenga que ejecutar en
  <em|background>, <verbatim|PShell> registra el <verbatim|pid> de
  <verbatim|PMandato> en una lista que constará de procesos en
  <em|background>, y continuará su ejecución sin esperar a la finalización de
  <verbatim|PMandato>, quien supervisará con normalidad el mandato que le
  hayan asignado. Además, <verbatim|PMandato> y sus hijos ignorarán las
  señales de <verbatim|SIGINT> y de <verbatim|SIGQUIT> de haberse lanzado en
  <em|background>.

  En cualquier caso, al finalizar <verbatim|PMandato>, se acciona la señal
  <verbatim|SIGCHLD>, de manera que <verbatim|PShell> actualiza la lista,
  marcando el nodo de su lista cuyo <verbatim|pid> corresponda con
  <verbatim|PMandato> como \SHecho\T.

  Independientemente de la línea insertada, antes de volverse a mostrar el
  <verbatim|prompt> y solicitarse una entrada de teclado nueva,
  <verbatim|PShell> limpiará la lista de procesos, mostrando por pantalla
  aquellos que hayan sido completados de forma análoga a como realiza Bash.

  En la sección de pseudocódigo se procede a explicar más concretamente cómo
  implemetnamos la comunicación de <verbatim|PMandato> con sus hijos.

  <subsubsection|Pseudocódigo>

  \;

  <subsection|Descripción de las principales funciones implementadas>

  <subsubsection|<verbatim|int escribirPrompt()>>

  Esta función se encarga de escribir un <em|prompt> personalizado teniendo
  en cuenta el directorio de trabajo actual. Devuelve 1, a no ser que no
  exista la variable de entorno <verbatim|HOME>, en cuyo caso devuelve
  <math|0>. Como consecuencia, abandona el bucle principal y finaliza nuestro
  minishell.

  <subsubsection|<verbatim|static void handler(int sig, siginfo_t* siginfo,
  void* context)>>

  Previamente, en la función <verbatim|main> se ha definido un
  <verbatim|struct sigaction>, con flags <verbatim|SA_SIGINFO> y
  <verbatim|SA_RESTART>, lo que nos permite obtener, respectivamente, mayor
  información y evitar la muerte del proceso receptor de la señal una vez
  ejecutado su manejador.

  Dentro de la función <verbatim|handler>, comprobamos que <verbatim|sig> se
  corresponda con la señal <verbatim|SIGCHLD>, lo cual nos indicaría la
  finalización de un hijo. Tras comprobar que el proceso que acciona la señal
  es el que comienza el tratamiento del mandato, su padre se encargaría de
  marcar a dicho proceso como hecho en su lista de procesos en
  <em|background>.

  <subsubsection|redirecciones (<verbatim|redirStdin>,
  <verbatim|redirStdout>, <verbatim|redirStderr>)>

  Son funciones que se encargan de gestionar los descriptores de fichero de
  aquellos procesos destinados a realizar mandatos. Esto es, el proceso raíz
  no aplica redirección alguna.

  <subsubsection|<verbatim|ejecutarComando(int i, tline* line)>>

  Esta función se encarga de lanzar el mandato especificado por
  <verbatim|tline>.

  <subsubsection|Funciones de control de lista de procesos>

  Para la correcta gestión de procesos en <em|background>, hemos ideado una
  lista de procesos, que se implementa como una lista con puntero al inicio y
  al final para lograr la mayor eficiencia posible de inserción de procesos,
  así como una rápida extracción de los mismos por medio del mandato
  implementado <verbatim|fg>.

  Las más relevantes, que escapan de la habitual implementación de una lista
  de estas características, son las siguientes:

  <\itemize>
    <item><verbatim|int limpiarLista(listaPIDInsercionFinal_t* L, int
    clasificar)>

    Esta función se encarga de eliminar mandatos en <em|background> que
    estuvieran marcados como hechos. Cuenta con un flag,
    <verbatim|clasificar>, cuya función nos permite diferenciar entre lo que
    se mostraría una vez acabado un mandato \Shabitual\T (una línea que
    muestra el mandato como realizado) y lo que se mostraría tras ejecutar
    <verbatim|jobs> (toda la lista de mandatos en <em|background>, estén
    realizados o no, en el orden en que fueron insertados en la lista).

    <item><verbatim|void borrarElementoPID(pid_t pid,
    listaPIDInsercionFinal_t* L)>

    Como los <verbatim|pid> de los procesos son únicos, nos podemos permitir,
    a la hora de eliminar un nodo de nuestra lista, comparar únicamente los
    <verbatim|pid>s, y eliminar el nodo correspondiente.

    <item><verbatim|void terminarElem(elem_t* elem)>

    Marcar el elemento dado como hecho (es decir, poner su flag de estado a
    cero).
  </itemize>

  <section|Comentarios personales>

  <subsection|Problemas encontrados>

  <\itemize>
    <item>En el segundo algoritmo que probamos antes de realizar este
    programa, intentamos utilizar exclusivamente dos <verbatim|pipe>s para
    intercomunicar un proceso hijo y un proceso padre. Llegados a cierto
    número de <verbatim|pipe>s, estas quedaban bloqueadas para lectura e
    impedían la ejecución del resto de mandatos, que se quedaban esperando a
    recibir entrada con la que trabajar.

    <item>Otro problema importante que hemos tenido ha sido el llamar, por
    medio de <verbatim|fg>, a un proceso en <em|background>. Hemos tenido
    problemas para acceder al <verbatim|pid> de un proceso que se encontraba
    en ejecución, dado que en nuestra lista de <verbatim|pid>s contábamos
    exclusivamente con aquellos de los procesos que se derivaban de forma
    directa del proceso principal.

    <item>Dentro de nuestros planteamientos en papel del minishell,
    necesitábamos conocer la señal del proceso que enviaba la señal al
    receptor, lo cual nos obligó a utilizar <verbatim|sigaction> en
    detrimento de <verbatim|signal>.

    <item>Modificar, desde el proceso raíz, la forma de actuar ante una señal
    por parte de un proceso que se encuentra ejecutando un mandato. Aunque,
    en papel, de nuevo, éramos capaces de solucionar el problema, se sucedían
    situaciones inesperadas y ante las que no encontramos respuesta; por
    ejemplo, dicho proceso moría inmediatamente tras mandarle una señal \Vpor
    ejemplo, <verbatim|SIGUSR1>\V, con un manejador distinto al por defecto.
  </itemize>

  En resumen, todos los problemas encontrados se derivan de una misma causa:
  habiendo diseñado y planteado en pseudocódigo el programa, siempre
  encontrábamos dificultades en la implementación. No obstante, la mayoría de
  estas dificultades las pudimos solventar, bien replanteando el código, bien
  utilizando soluciones alternativas.

  <subsection|Críticas constructivas y propuestas de mejora>

  <\itemize>
    <item>Estaría bien que, con el fin de aligerar la cantidad de texto en el
    código, y con fines puramente organizativos, pudiésemos implementar por
    nuestra cuenta archivos auxiliares (<verbatim|.h>, <verbatim|.c>), con
    los que, entre otras cosas, poder gestionar los tipos de datos más
    cómodamente, así como poder realizar pruebas aparte sin tener que
    comprometer con ello todo el código que había previamente.

    <item>Creemos que podríamos haber solucionado mucho antes algunos de los
    problemas que hemos tenido si hubiésemos conocido, de entrada, la
    existencia de herramientas como <verbatim|sigaction>, alternativas a
    <verbatim|signal> y que nos permitían mayor maniobrabilidad a la hora de
    solucionar problemas.

    <item>Una vez acabada nuestra práctica, nos dimos cuenta de que existían
    unas herramientas (<verbatim|tcsetpgrp>, <verbatim|setpgid>) que nos
    permitían gestionar todos los procesos cómodamente. Nuevamente, habría
    sido bueno que se nos indicase su existencia para acelerar el desarrollo
    de la práctica y poder hacerla lo mejor posible de entrada.

    <item>Un \Sjuez electrónico\T (como los de los concursos de programación)
    que evalúe la práctica para saber antes de entregar qué problemas tiene,
    y así poder asegurarnos la máxima nota sabiendo qué errores corregir
    antes de la corrección por parte de los profesores.
  </itemize>

  <subsection|Evaluación del tiempo dedicado>

  Pensamos que hemos invertido más tiempo del que nos gustaría, causado por
  problemas más de implementación (como los sucesos inesperados ya
  mencionados) que de diseño, lo que nos llevaría a replantear nuestro
  algoritmo en varias ocasiones.

  Nuevamente, también parte de este tiempo lo invertimos en investigar
  soluciones alternativas a los problemas que nos causaban las herramientas
  con las que contábamos de entrada. No obstante, esto nos ha permitido crear
  un mejor programa.

  <strong|Pseudocódigo>

  <\itemize>
    <item>Inicialización:

    <\itemize>
      \;

      <item>indicar al proceso que debe ignorar las señales
      <verbatim|SIGQUIT> y <verbatim|SIGINT>.
    </itemize>

    <item>Si recibimos una línea no vacía, distinguimos casos:

    <\itemize>
      <item>si la línea consta de un solo mandato, ejecutamos
      <verbatim|fork()>

      <\itemize>
        <item>el hijo pasa a atender por defecto a <verbatim|SIGQUIT> y
        <verbatim|SIGINT> y ejecuta el mandato pedido.
      </itemize>

      <\itemize>
        <item>El padre espera al hijo y devuelve su código de error.
      </itemize>
    </itemize>

    <\itemize>
      <item>si la línea consta de más de un comando
    </itemize>

    <item>Formar <verbatim|pipes>

    <\itemize>
      <item><verbatim|malloc()> para \Sinicializar\T puntero de
      <verbatim|pipe>s

      <item><verbatim|malloc()> para \Sinicializar\T tantos <verbatim|pipe>s
      como mandatos haya
    </itemize>

    <item>Bucle <verbatim|exec()>: tantas iteraciones como número de mandatos
    haya

    <\enumerate-roman>
      <item><verbatim|fork()>

      <\itemize>
        <item>Hijo: distinguimos casos

        <\enumerate-alpha>
          <item>si es la primera iteración:

          <\itemize>
            <item>cerrar <verbatim|pipe[iteración][0]>, ya que no lo vamos a
            utilizar

            <item>duplicar <verbatim|pipe[iteración][1]> en <verbatim|stdout>

            <item>cerrar <verbatim|pipe[iteración][1]>, que ha quedado
            abierto tras la duplicación

            <item><verbatim|exec()>
          </itemize>

          <item>si no es ni la primera ni la última iteración:

          <\itemize>
            <item>cerrar <verbatim|pipe[iteración-1][1]>, ya que no lo vamos
            a utilizar

            <item>cerrar <verbatim|pipe[iteración][0]>, ya que no lo vamos a
            utilizar

            <item>duplicar <verbatim|pipe[iteración-1][0]> en
            <verbatim|stdin>

            <item>cerrar <verbatim|pipe[iteración-1][0]>, ya que no lo vamos
            a utilizar

            <item>duplicar <verbatim|pipe[iteración][1]> en <verbatim|stdout>

            <item>cerrar <verbatim|pipe[iteración][1]> ya que no lo vamos a
            utilizar

            <item><verbatim|exec()>
          </itemize>

          <item>si es la última iteración:

          <\itemize>
            <item>cerrar <verbatim|pipe[iteración-1][1]>, ya que no lo vamos
            a utilizar

            <item>cerrar <verbatim|pipe[iteración][0]>, ya que no lo vamos a
            utilizar (esto no existe)

            <item>duplicar <verbatim|pipe[iteración-1][0]> en
            <verbatim|stdin>

            <item>cerrar <verbatim|pipe[iteración-1][0]>, ya que no lo vamos
            a utilizar

            <item><verbatim|exec()>

            Nota: no tenemos que modificar <verbatim|stdout> para que la
            última iteración funcione correctamente, dado que ha heredado los
            descriptores de fichero del padre, que indicaban que
            <verbatim|stdout> estaba redirigido (o no) previamente a un
            fichero.
          </itemize>
        </enumerate-alpha>
      </itemize>
    </enumerate-roman>
  </itemize>

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;

  \;
</body>

<\initial>
  <\collection>
    <associate|page-breaking|professional>
    <associate|page-crop-marks|a4>
    <associate|page-medium|paper>
  </collection>
</initial>

<\references>
  <\collection>
    <associate|auto-1|<tuple|1|1>>
    <associate|auto-10|<tuple|2.3.3|1>>
    <associate|auto-11|<tuple|2.3.4|?>>
    <associate|auto-12|<tuple|2.3.5|?>>
    <associate|auto-13|<tuple|3|?>>
    <associate|auto-14|<tuple|3.1|?>>
    <associate|auto-15|<tuple|3.2|?>>
    <associate|auto-16|<tuple|3.3|?>>
    <associate|auto-2|<tuple|2|1>>
    <associate|auto-3|<tuple|2.1|1>>
    <associate|auto-4|<tuple|2.2|1>>
    <associate|auto-5|<tuple|2.2.1|1>>
    <associate|auto-6|<tuple|2.2.2|1>>
    <associate|auto-7|<tuple|2.3|1>>
    <associate|auto-8|<tuple|2.3.1|1>>
    <associate|auto-9|<tuple|2.3.2|1>>
  </collection>
</references>

<\auxiliary>
  <\collection>
    <\associate|toc>
      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|1<space|2spc>Índice
      de contenidos> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-1><vspace|0.5fn>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|2<space|2spc>Descripción
      del código> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-2><vspace|0.5fn>

      <with|par-left|<quote|1tab>|2.1<space|2spc>Funcionalidad implementada
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-3>>

      <with|par-left|<quote|1tab>|2.2<space|2spc>Pseudocódigo y planteamiento
      del programa <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-4>>

      <with|par-left|<quote|1tab>|2.3<space|2spc>Descripción de las
      principales funciones implementadas
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-5>>

      <vspace*|1fn><with|font-series|<quote|bold>|math-font-series|<quote|bold>|3<space|2spc>Comentarios
      personales> <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-6><vspace|0.5fn>

      <with|par-left|<quote|1tab>|3.1<space|2spc>Problemas encontrados
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-7>>

      <with|par-left|<quote|1tab>|3.2<space|2spc>Críticas constructivas
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-8>>

      <with|par-left|<quote|1tab>|3.3<space|2spc>Propuesta de mejoras
      <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-9>>

      <with|par-left|<quote|1tab>|3.4<space|2spc>Evaluación del tiempo
      dedicado <datoms|<macro|x|<repeat|<arg|x>|<with|font-series|medium|<with|font-size|1|<space|0.2fn>.<space|0.2fn>>>>>|<htab|5mm>>
      <no-break><pageref|auto-10>>
    </associate>
  </collection>
</auxiliary>