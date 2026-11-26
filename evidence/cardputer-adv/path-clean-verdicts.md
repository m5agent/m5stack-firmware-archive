| Case | Verdict | Evidence |
| --- | --- | --- |
| display | PASS | colored circles and rectangles change from 3 to 8 s |
| quickstart-display | PASS | colored circles and rectangles change from 3 to 8 s |
| keyboard | PASS | space, Backspace and Enter through FIFO; Fn+Del deletes |
| button | PASS | three press/release pairs via numeric pin, btn and g0 |
| ir_nec | PASS | serial and waveform agree for 0x1111/0x34..0x3a |
| battery | PASS | 3700 mV / 50% to 4000 mV / 87% from ADC |
| imu | PASS | two injected accel/gyro vectors displayed |
| speaker | PASS | four 7000 Hz and four 4000 Hz bursts on I2S1 |
| mic | PASS | REC reacts to input, G0 replays 733.0 Hz then REC resumes |
| sdcard | PASS | FAT32 create/append/rename/delete, 1 MiB read/write, on-disk data verified |
| speaker-sd-wav | PASS | SPI SD source and I2S1 capture agree for all 12 alternating 440/660 Hz seconds |

11/11 passed
