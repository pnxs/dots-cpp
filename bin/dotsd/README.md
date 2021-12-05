# Application: dotsd

The *dotsd (DOTS daemon)* is a standalone *DOTS host* (i.e "server") application. Outside of advanced use cases, it is usually the only host application required to set up a running DOTS system.

A DOTS host acts in the role of a *message broker* and distributes published objects to all subscribed *DOTS guests* (i.e. "clients"). Guests can establish connections through a variety of transport protocols (e.g. TCP), depending on how the host is configured.

In addition to the usual host functionalities, the dotsd will also itself publish additional information via objects of the following *internal* types:
*  `DotsClient`: Status information for each connected guest.
*  `DotsDaemonStatus`: Status information for the daemon itself.

# Usage

The dotsd can use a variety of transport protocols, which can be configured by providing URI-like endpoints:

```sh
# listen only on TCP endpoint at localhost address using the default port
dotsd
dotsd --listen=tcp://127.0.0.1
dotsd --listen=tcp://127.0.0.1:11234

# listen only on UNIX domain socket endpoint at path "/tmp/dots.socket"
# (requires corresponding OS support)
dotsd --listen=uds:/tmp/dots.socket

# listen only on WebSocket endpoint at localhost address using custom port
dotsd --listen=ws://127.0.0.1:11235

# listen on multiple endpoints simultaneously
dotsd --listen=tcp://127.0.0.1:11234 --listen=uds:/tmp/dots.socket
```

If a guest application is based on the `dots::Application` class of the dots-cpp library, it can connect to the dotsd (or any DOTS host) by providing the corresponding host endpoint as an argument:

```sh
# open host connection via TCP endpoint at localhost address using the default port
some-app
some-app --open=tcp://127.0.0.1
some-app --open=tcp://127.0.0.1:11234

# open host connection via UNIX domain socket endpoint at path "/tmp/dots.socket"
# (requires corresponding OS support)
some-app --open=uds:/tmp/dots.socket

# open host connection via WebSocket endpoint at remote address using custom port
some-app --open=ws://192.168.0.42:11235
```
