package legend

import (
	"os"
	"testing"
)

func TestBanner(t *testing.T) {
	os.Remove("./test/banner_test.png")
	fout, err := os.Create("./test/banner_test.png")
	if err != nil {
		t.Fatal(err)
	}
	err = Banner(fout, 0, 0, "Hello, World!")
	if err != nil {
		t.Error(err)
	}
}
