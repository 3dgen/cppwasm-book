default:
	gitbook build

pdf:
	gitbook pdf

server:
	go run server.go

cover:
	convert -resize 1800x2360! cover.png cover.jpg
	convert -resize 200x262!   cover.png cover_small.jpg


# https://3dgen.cn/cppwasm-book
deploy:
	-rm -rf _book
	gitbook build

	cd _book && \
		git init && \
		git add . && \
		git commit -m "Update github pages" && \
		git push --force --quiet "https://github.com/3dgen/cppwasm-book.git" master:gh-pages

	@echo deploy done

clean:
	-rm -rf _book
