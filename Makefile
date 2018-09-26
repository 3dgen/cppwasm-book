default:
	gitbook build

server:
	go run server.go

clean:
	-rm -rf _book
