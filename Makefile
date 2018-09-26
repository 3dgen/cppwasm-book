default:
	gitbook build

pdf:
	gitbook pdf

server:
	go run server.go

cover:
	convert -resize 1800x2360! cover.png cover.jpg
	convert -resize 200x262!   cover.png cover_small.jpg

clean:
	-rm -rf _book
