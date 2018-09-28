default:
	gitbook build

pdf:
	gitbook pdf

server:
	go run server.go

cover:
	convert -resize 1800x2360! cover.png cover.jpg
	convert -resize 200x262!   cover.png cover_small.jpg


# https://chai2010.cn/cppwasm-book
deploy:
	-rm -rf _book
	gitbook build

	cd _book && \
		git init && \
		git add . && \
		git commit -m "Update github pages" && \
		git push --force --quiet "https://github.com/chai2010/cppwasm-book.git" master:gh-pages

	@echo deploy done

clean:
	-rm -rf _book
