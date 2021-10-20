# LWIP Echo Test

This is an extremely basic demo application that uses the LWIP stack via PCAP to echo anything sent to it.

## Usage

First of all build the docker image:

```sh
sudo docker build -t lwip-test .
```

Then run the newly created docker image:

```sh
docker run --ti lwip-test
```

This will stay running until cancelled with Ctrl-C. In a new terminal, connect to the test service. The IP is hard coded to `172.17.0.5` and the port to `11111`:

```sh
nc 172.17.0.5 11111
```

Anything you type into this console will echo on the other one. For example, if you type "hello world" you will see:

```
echo_init: pcb: e42f6d80
echo_init: tcp_bind: 0
echo_init: listen-pcb: e42f6e60
echo_msgaccept called
Got: hello world
```

You can exit the app and nc with Ctrl-C.

## Notes

The `lwip-include/arch` directory is a copy of the lwIP directory from `contrib/ports/unix/port/include/arch`.
