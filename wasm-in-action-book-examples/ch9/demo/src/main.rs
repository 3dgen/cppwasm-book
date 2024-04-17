
fn main() {
	println!("{:?}", std::env::args());

	for (i, x) in std::env::args().enumerate() {
		println!("arg[{}]: {}", i, x);
	}
}
