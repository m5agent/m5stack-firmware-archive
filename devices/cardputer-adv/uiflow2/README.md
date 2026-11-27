# Cardputer-ADV UIFlow2 source examples

The source entries preserve the bytes sent over USB CDC and the native
virtual-device results on stock UIFlow2 v2.5.3. Two keyboard examples, six LoRa
and GPS examples, and two CC1101 RF examples passed. The published NFC sketch
reached a constructor error when the stock CAP implementation passed
`chip=` to `NFCUnit`. The compatibility variant passed with two synthetic
tag formats. [Per-case results](../../../evidence/cardputer-adv/uiflow2-cap-summary.json)

The stock manifests pin the official M5Burner downloads for
[v2.5.3](v2.5.3/firmware.json) and the NFC follow-up on
[v2.5.5](v2.5.5/firmware.json). Fetch the image for the selected run and verify
its SHA-256 before replaying a sketch.
The [simulator source](https://github.com/m5agent/esp32sim/tree/89318bb6621ed4af75ec11b6465c3a39a4ec95ca)
and each case manifest identify the runtime used for those results. The
[programming guide](https://uiflow-micropython.readthedocs.io/en/latest/controllers/cardputer-adv.html)
describes the keyboard interface.
