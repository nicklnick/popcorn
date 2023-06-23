# Popcorn POP3 Server

- Saul Castañeda n°62493
- Juan Burda n°62094
- Elian Paredes n°62504
- Nicolás Margenat n°62028
  
## Importante
- El servidor por default corre en el puerto 1110. Este valor se puede cambiar al ejecutar el server por parámetro.
- La aplicación que implementa el protocolo Popcorn corre en el puerto 2882. Este valor no se puede cambiar.

## Compilación
Para crear los archivos ejecutables `server` y `client`, se debe ejecutar el comando `make all` en carpeta raíz del proyecto.
Luego, los archivos generados se encontrarán dentro del directorio `./bin`.

## Ejecución
### Ejecución del Server
Para ejecutar el server se debe correr los siguientes comandos dentro del directorio `bin`:
```bash
./server [-p <port>] -d <mail_dir> -a <user>:<pass> [[-u <user>:<pass>]...]
```
donde: 
- `d <mail_dir>` es el directorio de mails del servidor. Aquí dentro se encontrarán los directorios de los distintos usuarios.
- `-a <user>:<pass>` es el usuario y contraseña del administrador del servidor. Estas credenciales son las que se deben mandar siempre que se manden requests con el cliente desarrollado.
- `-u <user>:<pass>` son los distintos usuarios que hay en el servidor. En esta opción se deben incluir TODOS los usuarios que aparecen dentro del directorio mails y asignarles una contraseña.
- `-p <port>` es el puerto donde correrá el servidor. Si no se especifica, su valor default es 1110.

### Ejecución del Client
Para ejecutar el cliente se debe correr el siguiente comando:
```bash
./client -a <user>:<password> <command> [args]
```
donde `-a <user>:<password>` es el usuario y contraseña del administrador del server. Este debió haber sido especificado al ejecutar el server.
Además para saber qué comandos están disponibles ejecutar (para el campo <command>):
```bash
./client -h
```

## Estructura del directorio de mails
Los directorios de los usuarios deben estar dentro de la carpeta que se especifique con la opción `-d` al ejecutar `./server`.
Dicho directorio debe contener únicamente directorios (los correspondientes a los usuarios). Asimismo, los directorios de los usuarios deben contener únicamente archivos.
Por ejemplo, una estructura válida sería la siguiente:
```bash
mail
├── nico
│   ├── News
│   ├── Subscription
│   └── Youtube
└── saul
    ├── Decks
    ├── ITBA
    └── mail
```

## Estructura del proyecto
El proyecto se estructura de la siguiente manera.
```bash
popcorn
└── src
    ├── client
    └── server
        ├── buffer
        ├── parser
        ├── popcorn
        ├── selector
        ├── session
        ├── sm
        └── utils
```
- Dentro de `src` se encuentra todo el código fuente.
- `client` contiene el código desarrollado para la aplicación cliente.
- `server` contiene todo el código relacionado con el servidor.
    - `buffer` contiene el buffer de escritura/lectura provisto por la cátedra.
    - `parser` contiene el parser de comandos del servidor.
    - `popcorn` contiene el código relacionada a la aplicación que implementa el protocolo Popcorn.
    - `selector` contiene el código del selector provisto por la cátedra.
    - `session` contiene el código que modela el objeto session, el cual representa una conexión con un cliente.
    - `sm` contiene el código relacionado a la máquina de estados.
    - `utils` distintas funciones útiles que se utilizan en los otros directorios.
