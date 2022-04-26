# Memory Game

Um simples Jogo da Memória desenvolvido com a linguagem C e SDL 2 para Linux, Android, Windows, e Web (WebAssembly).

100% desenvolvido em C.

## Informações

Desenvolvido com:

* Linguagem de programação C;
* SDL 2

Compatível com:

* Linux;
* Android;
* Windows;
* Navegador Web (WebAssembly)

## Acessar versão online (WebAssembly)

Em breve...

## Compilar

Para Linux, será preciso compilar a partir do código fonte, para isso, basta instalar as dependencias de desenvolvimento do ```SDL2```, ```SDL2_image```, ```SDL2_ttf```, assim como ```GCC``` e ```cmake```. Depois:

```
cmake .
make
```

Para WebAssembly, será preciso instalar o ```Emscripten``` ([emscripten.org](https://emscripten.org/)), depois:

```
emcmake cmake .
make
```

## Imagens

![Screenshot](/screenshots/screenshot_01.png?raw=true)

## Licença

MIT License

Copyright (c) 2022 Fábio Pichler
