fn main() {
    std::process::Command::new("make")
        .stdin(std::process::Stdio::inherit())
        .stdout(std::process::Stdio::inherit())
        .stderr(std::process::Stdio::inherit())
        .args(&["myclib"])
        .output()
        .unwrap();
}
