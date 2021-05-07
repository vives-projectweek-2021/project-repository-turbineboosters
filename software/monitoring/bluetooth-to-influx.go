package main

import (
	"fmt"
	"strconv"
	"strings"
	"syscall"
	"os"

	influxdb2 "github.com/influxdata/influxdb-client-go"
	"golang.org/x/sys/unix"
)

func main() {
  token := os.Getenv("INFLUX_TOKEN")
	client := influxdb2.NewClient("http://152.228.212.33:8186", token)

	Connect("20:17:03:06:55:74", &client)

	defer client.Close()
}

func insert_data(payload string, client influxdb2.Client) {
	const bucket = "data"
	const org = "turbineboosters"
	writeAPI := client.WriteAPI(org, bucket)

	b := ""
	for _, c := range payload {
		if int(c) != 0 && int(c) != 13 {
			b += string(c)
		}
	}

	r, _ := strconv.ParseInt("0x"+b[0:4], 0, 64)
	power := float64(r) / 100

	r, _ = strconv.ParseInt("0x"+b[4:8], 0, 64)
	amp := float64(r) / 100

	r, _ = strconv.ParseInt("0x"+b[8:12], 0, 64)
	volt := float64(r) / 100

	r, _ = strconv.ParseInt("0x"+b[12:16], 0, 64)
	temp := float64(r) / 100

	r, _ = strconv.ParseInt("0x"+b[16:20], 0, 64)
	humid := float64(r) / 100

	writeAPI.WriteRecord(fmt.Sprintf("power,unit=watt value=%f", power))
	writeAPI.WriteRecord(fmt.Sprintf("temperature,unit=degree value=%f", temp))
	writeAPI.WriteRecord(fmt.Sprintf("humidity,unit=percentage value=%f", humid))
	writeAPI.WriteRecord(fmt.Sprintf("voltage,unit=volt value=%f", volt))
	writeAPI.WriteRecord(fmt.Sprintf("ampere,unit=ampere value=%f", amp))
	writeAPI.Flush()
}

func Connect(macAddress string, client *influxdb2.Client) {
	mac := str2ba(macAddress)

	fd, err := unix.Socket(syscall.AF_BLUETOOTH, syscall.SOCK_STREAM, unix.BTPROTO_RFCOMM)
	check(err)
	addr := &unix.SockaddrRFCOMM{Addr: mac, Channel: 1}

	fmt.Println("connecting...")
	err = unix.Connect(fd, addr)
	check(err)
	defer unix.Close(fd)
	fmt.Println("done")

	payload := ""

	for {
		var data = make([]byte, 16)
		unix.Read(fd, data)

		if len(data) > 0 {
			payload += string(data)
		}

		for i, c := range payload {
			if int(c) == 10 && len(payload) > 1 {
				go insert_data(payload[:i], *client)
				payload = ""
			}
		}

	}
}

func str2ba(addr string) [6]byte {
	a := strings.Split(addr, ":")
	var b [6]byte
	for i, tmp := range a {
		u, _ := strconv.ParseUint(tmp, 16, 8)
		b[len(b)-1-i] = byte(u)
	}
	return b
}

func check(err error) {
	if err != nil {
		panic(err)
	}
}
