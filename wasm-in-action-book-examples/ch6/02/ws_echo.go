package main

import (
	"fmt"
	"log"
	"net/http"
	"syscall"

	"golang.org/x/net/websocket"

	"os"
	"os/signal"
)

func main() {
	log.Println("ws_echo start...")

	wsPort := 40001
	go func() {
		log.Println(fmt.Sprint("WebSocket:", wsPort, " Listening ..."))
		http.Handle("/ws_echo", websocket.Handler(webSocketHandler))
		err := http.ListenAndServe(fmt.Sprint(":", wsPort), nil)
		if err != nil {
			panic("ListenAndServe: " + err.Error())
		}
	}()

	httpPort := 80
	go func() {
		log.Println(fmt.Sprint("http:", httpPort, " Listening ..."))
		err := http.ListenAndServe(fmt.Sprint(":", httpPort),
			http.FileServer(http.Dir("./")))
		if err != nil {
			panic("ListenAndServe: " + err.Error())
		}
	}()

	ch := make(chan os.Signal, 1)
	signal.Notify(ch, syscall.SIGINT, syscall.SIGTERM)
	log.Printf("ws_echo quit (%v)\n", <-ch)
}

func webSocketHandler(ws *websocket.Conn) {
	ws.PayloadType = websocket.TextFrame
	defer ws.Close()

	rtemp := make([]byte, 32768)
	for {
		n, err := ws.Read(rtemp)
		if err != nil {
			log.Println("Error:Read:", err)
			return
		}

		n, err = ws.Write(rtemp[:n])
	}
}
