package main

import (
	"archive/zip"
	"bytes"
	"encoding/binary"
	"io"
	"os"
	"runtime"
	"sync"
	"time"
	"unsafe"

	"./legend"
	"./thredds"
)

//#define NINJA_OK    0
//#define NINJA_IOERR 1
//
//#define NINJA_CS_DEFAULT      "default"
//#define NINJA_CS_BLUES        "blues"
//#define NINJA_CS_GREENS       "greens"
//#define NINJA_CS_PINKS        "pinks"
//#define NINJA_CS_MAGICBEANS   "magic_beans"
//#define NINJA_CS_PINKTOGREEN  "pink_to_green"
//#define NINJA_CS_ROPGW        "ropgw"
//#include <stdlib.h>
//#include <time.h>
//typedef struct tm tm;
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

// ninja_exec_path returns the path of WindNinja.  The result must be free'd by
// the caller.
//
//export ninja_exec_path
func ninja_exec_path() *C.char {
	x, err := os.Executable()
	if err != nil {
		return nil
	}
	return C.CString(x)
}

// ninja_draw_legend creates a PNG legend for the KMZ output.  It needs the
// breaks between the colors, and the scheme as defined above.  Note if a color
// scheme is added, it must be added in legend/legend.go and here.
//
// See legend.DrawLegend for documentation
//export ninja_draw_legend
func ninja_draw_legend(path *C.char, breaks *C.double, n C.int, scheme *C.char, units *C.char) C.int {
	f, err := os.Create(C.GoString(path))
	if err != nil {
		return 1
	}
	defer f.Close()
	s := arrayToSlice(breaks, n)
	if s == nil {
		return C.int(2)
	}
	err = legend.DrawLegend(f, s, C.GoString(scheme), C.GoString(units))
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
	f, err := os.Create(C.GoString(path))
	if err != nil {
		return 1
	}
	defer f.Close()
	d := time.Hour * time.Duration(hours)
	err = thredds.Download(f, C.GoString(name), float64(minX), float64(maxX), float64(minY), float64(maxY), d)
	if err != nil {
		return 1
	}
	return 0
}

func goTimeToC(nt time.Time, t *C.tm) {
	if t != nil {
		t.tm_sec = C.int(nt.Second())
		t.tm_min = C.int(nt.Minute())
		t.tm_hour = C.int(nt.Hour())
		t.tm_mday = C.int(nt.Day())
		t.tm_mon = C.int(nt.Month())
		t.tm_year = C.int(nt.Year())
		t.tm_wday = C.int(nt.Weekday())
		t.tm_yday = C.int(nt.YearDay())
		t.tm_isdst = C.int(0)
	}
}

func cTimeToGo(t *C.tm, loc *time.Location) time.Time {
	nt := time.Date(
		int(t.tm_year),
		time.Month(t.tm_mon),
		int(t.tm_mday),
		int(t.tm_hour),
		int(t.tm_min),
		int(t.tm_sec),
		0, loc)
	return nt
}

// ninja_parse_time parses a string and populates a struct tm
//
//export ninja_parse_time
func ninja_parse_time(layout, value *C.char, t *C.tm) C.int {
	x, y := C.GoString(layout), C.GoString(value)
	nt, err := time.Parse(x, y)
	if err != nil {
		return 1
	}
	goTimeToC(nt, t)
	return 0
}

// ninja_format_time formats a tm into the format provides, returning a string
// that should be free'd by the caller.
//
//export ninja_format_time
func ninja_format_time(layout *C.char, t *C.tm) *C.char {
	nt := cTimeToGo(t, time.UTC)
	s := nt.Format(C.GoString(layout))
	return C.CString(s)
}

var (
	zoneMu   sync.RWMutex
	zoneMap  = map[string]*time.Location{}
	zoneOnce sync.Once
	zoneZip  *zip.ReadCloser
)

func loadLocationFromZip(z string) (*time.Location, error) {
	zoneOnce.Do(func() {
		var err error
		zoneZip, err = zip.OpenReader("zoneinfo.zip")
		if err != nil {
			panic(err)
		}
	})
	zoneMu.RLock()
	loc, ok := zoneMap[z]
	zoneMu.RUnlock()
	if ok {
		return loc, nil
	}
	for _, f := range zoneZip.File {
		if f.Name == z {
			rc, err := f.Open()
			if err != nil {
				return nil, err
			}
			b := &bytes.Buffer{}
			_, err = io.Copy(b, rc)
			rc.Close()
			loc, err = time.LoadLocationFromTZData(z, b.Bytes())
			if err != nil {
				return nil, err
			}
			zoneMu.Lock()
			zoneMap[z] = loc
			zoneMu.Unlock()
			break
		}
	}
	return loc, nil
}

func loadLocation(z string) (*time.Location, error) {
	if runtime.GOROOT() == "" {
		return loadLocationFromZip(z)
	}
	zoneMu.RLock()
	loc, ok := zoneMap[z]
	zoneMu.RUnlock()
	if ok {
		return loc, nil
	}
	loc, err := time.LoadLocation(z)
	if err != nil {
		return nil, err
	}
	zoneMu.Lock()
	zoneMap[z] = loc
	zoneMu.Unlock()
	return loc, nil
}

// ninja_time_in changes a struct tm from src time zone to dst time zone in
// place.
//
//export ninja_time_in
func ninja_time_in(src, dst *C.char, t *C.tm) C.int {
	srcZ, err := loadLocation(C.GoString(src))
	if err != nil {
		return 1
	}
	dstZ, err := loadLocation(C.GoString(dst))
	if err != nil {
		return 1
	}
	if srcZ == dstZ {
		return 0
	}
	nt := cTimeToGo(t, srcZ)
	dt := nt.In(dstZ)
	goTimeToC(dt, t)
	return 0
}

func main() {}
