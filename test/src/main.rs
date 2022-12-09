use core::time::Duration;
use std::env;
use tokio::{io::AsyncWriteExt, task, time};
use tokio_serial::{DataBits, FlowControl, Parity, SerialPortBuilderExt, StopBits};

#[cfg(unix)]
const DEFAULT_TTY: &str = "/dev/ttyUSB0";
#[cfg(windows)]
const DEFAULT_TTY: &str = "COM1";

#[tokio::main]
async fn main() -> tokio_serial::Result<()> {
    let mut args = env::args();
    let tty_path = args.nth(1).unwrap_or_else(|| DEFAULT_TTY.into());

    let mut port = tokio_serial::new(tty_path, 1200)
        .flow_control(FlowControl::None)
        .data_bits(DataBits::Seven)
        .stop_bits(StopBits::One)
        .parity(Parity::Even)
        .open_native_async()?;

    #[cfg(unix)]
    port.set_exclusive(false)
        .expect("Unable to set serial port exclusive to false");

    // Sending task. Only send fields we are interested in.
    let frame_task = task::spawn(async move {
        let mut interval = time::interval(Duration::from_secs(2));

        let mut base_val = 0u32;
        let mut hchc_val = 1111u32;
        let mut hchp_val = 2222u32;
        let mut iinst_val = 120u16;
        let mut papp_val = 16254u32;

        loop {
            interval.tick().await;

            let checksummable = format!("BASE{}{:09}", '\u{0020}', base_val);
            let checksum = compute_checksum(&checksummable);
            let base = format!("{}{}{}{}{}", '\u{000a}', checksummable, '\u{0020}', checksum, '\u{000d}');
            let checksummable = format!("HCHC{}{:09}", '\u{0020}', hchc_val);
            let checksum = compute_checksum(&checksummable);
            let hchc = format!("{}{}{}{}{}", '\u{000a}', checksummable, '\u{0020}', checksum, '\u{000d}');
            let checksummable = format!("HCHP{}{:09}", '\u{0020}', hchp_val);
            let checksum = compute_checksum(&checksummable);
            let hchp = format!("{}{}{}{}{}", '\u{000a}', checksummable, '\u{0020}', checksum, '\u{000d}');
            let checksummable = format!("IINST{}{:03}", '\u{0020}', iinst_val);
            let checksum = compute_checksum(&checksummable);
            let iinst = format!("{}{}{}{}{}", '\u{000a}', checksummable, '\u{0020}', checksum, '\u{000d}');
            let checksummable = format!("PAPP{}{:05}", '\u{0020}', papp_val);
            let checksum = compute_checksum(&checksummable);
            let papp = format!("{}{}{}{}{}", '\u{000a}', checksummable, '\u{0020}', checksum, '\u{000d}');

            let frame = format!("{}{}{}{}{}{}{}", '\u{0002}', base, hchc, hchp, iinst, papp, '\u{0003}');
            port.write(frame.as_bytes()).await.ok();

            base_val += 1;
            hchc_val += 1;
            hchp_val += 1;
            iinst_val += 1;
            papp_val += 1;
        }
    });

    frame_task.await.ok();

    Ok(())
}

fn compute_checksum(content: &String) -> char {
    let summed = content.bytes().fold(0u32, |acc, c| acc + u32::from(c)) & 0x3f;
    char::from(summed as u8 + 0x20)
}
