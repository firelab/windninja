package main

import (
	"bytes"
	"encoding/binary"
	"os"
	"time"
	"unsafe"

	"./legend"
	"./thredds"
)

//#define NINJA_OK    0
//#define NINJA_IOERR 1
//
//#define NINJA_CS_DEFAULT      0
//#define NINJA_CS_BLUES        1
//#define NINJA_CS_GREENS       2
//#define NINJA_CS_PINKS        3
//#define NINJA_CS_MAGICBEANS   4
//#define NINJA_CS_PINKTOGREEN  5
//#define NINJA_CS_ROPGW        6
//#include <stdlib.h>
import "C"

func arrayToSlice(data *C.double, n C.int) []float64 {
	s := make([]float64, int(n))
	size := int(unsafe.Sizeof(*data))
	b := C.GoBytes(unsafe.Pointer(data), C.int(size)*n)
	buf := bytes.NewBuffer(b)
	err := binary.Read(buf, binary.LittleEndian, &s)
	if err != nil {
		return nil
	}
	return s
}

// ninja_draw_legend creates a PNG legend for the KMZ output.  It needs the
// breaks between the colors, and the scheme as defined above.  Note if a color
// scheme is added, it must be added in legend/legend.go and here.
//
// See legend.DrawLegend for documentation
//export ninja_draw_legend
func ninja_draw_legend(path *C.char, breaks *C.double, n C.int, scheme C.int, units *C.char) C.int {
	p := C.GoString(path)
	f, err := os.Create(p)
	if err != nil {
		return 1
	}
	defer f.Close()
	s := arrayToSlice(breaks, n)
	if s == nil {
		return C.int(2)
	}
	u := C.GoString(units)
	err = legend.DrawLegend(f, s, int(scheme), u)
	if err != nil {
		return 1
	}
	return 0
}

// ninja_thredds_download fetches a forecast file from the UCAR thredds data
// server.
//
// See thredds.Download for documentation
//export ninja_thredds_download
func ninja_thredds_download(path *C.char, name *C.char, minX, maxX, minY, maxY C.double, hours C.int) C.int {
	p := C.GoString(path)
	f, err := os.Create(p)
	if err != nil {
		return 1
	}
	defer f.Close()
	n := C.GoString(name)
	d := time.Hour * time.Duration(hours)
	err = thredds.Download(f, n, float64(minX), float64(maxX), float64(minY), float64(maxY), d)
	if err != nil {
		return 1
	}
	return 0
}

func main() {}
