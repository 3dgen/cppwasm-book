fn main() {
	print!("< ");
	for s in std::env::args().skip(1) {
		print!("{} ", s);
	}

	println!(r#">
   \
    \
        .--.
       |o_o |
       |:_/ |
      //   \ \
     (|     | )
    /'\_   _/`\
    \___)=(___/
"#);
}
