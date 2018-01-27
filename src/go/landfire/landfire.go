package main

import (
	"archive/zip"
	"bytes"
	"encoding/json"
	"encoding/xml"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

type requestServiceResponse struct {
	Response struct {
		Piece []struct {
			DownloadURL  string `json:"DOWNLOAD_URL"`
			ThumbnailURL string `json:"THUMBNAIL_URL"`
		} `json:"PIECE"`
		Status bool `json:"STATUS"`
	} `json:"REQUEST_SERVICE_RESPONSE"`
}

type landfireResponse struct {
	XMLName xml.Name
	Return  string `xml:"return"`
}

func baseURL() *url.URL {
	return &url.URL{
		Scheme: "https",
		Host:   "landfire.cr.usgs.gov",
	}
}

// returnValue extracts the generic 'return' node from the response
func returnValue(r *http.Response) (string, error) {
	var resp landfireResponse
	err := xml.NewDecoder(r.Body).Decode(&resp)
	if err != nil {
		return "", err
	}
	return resp.Return, nil
}

// requestValidation sends a generated request to the validation service to see
// if we even have data.  The only information we need is the envelope and the
// product.  The url download URL is returned on success, otherwise an empty
// string and the appropriate error.
func requestValidation(e Envelope, product string) (*url.URL, error) {
	u := baseURL()
	u.Path = "/requestValidationServiceClient/sampleRequestValidationServiceProxy/processAOI.jsp"
	q := url.Values{}
	q.Set("TOP", fmt.Sprintf("%f", e.MaxY))
	q.Set("BOTTOM", fmt.Sprintf("%f", e.MinY))
	q.Set("LEFT", fmt.Sprintf("%f", e.MinX))
	q.Set("RIGHT", fmt.Sprintf("%f", e.MaxX))
	q.Set("CHUNK_SIZE", "250")
	q.Set("LAYER_IDS", product)
	q.Set("JSON", "true")
	u.RawQuery = q.Encode()

	resp, err := http.Get(u.String())
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	if err != nil {
		return nil, err
	}
	var sResp requestServiceResponse
	b, _ := ioutil.ReadAll(resp.Body)
	if err := json.NewDecoder(bytes.NewReader(b)).Decode(&sResp); err != nil {
		//if err := json.NewDecoder(resp.Body).Decode(&sResp); err != nil {
		return nil, err
	}
	if len(sResp.Response.Piece) < 1 {
		fmt.Printf("%+v", string(b))
		return nil, fmt.Errorf("invalid response")
	}
	return url.Parse(sResp.Response.Piece[0].DownloadURL)
}

func initiateDownload(id string) error {
	u := baseURL()
	u.Path = "/axis2/services/DownloadService/initiateDownload"
	q := url.Values{}
	q.Set("downloadID", id)
	u.RawQuery = q.Encode()

	resp, err := http.Get(u.String())
	if err != nil {
		return err
	}
	resp.Body.Close()
	return err
}

func downloadStatus(id string) (int, error) {
	u := baseURL()
	u.Path = "/axis2/services/DownloadService/getDownloadStatus"
	q := url.Values{}
	q.Set("downloadID", id)
	u.RawQuery = q.Encode()
	resp, err := http.Get(u.String())
	if err != nil {
		return 0, err
	}
	rv, err := returnValue(resp)
	resp.Body.Close()
	if err != nil {
		return 0, err
	}
	tkns := strings.Split(rv, ",")
	return strconv.Atoi(tkns[0])
}

func getData(id string) (string, error) {
	u := baseURL()
	u.Path = "/axis2/services/DownloadService/getData"
	q := url.Values{}
	q.Set("downloadID", id)
	u.RawQuery = q.Encode()
	resp, err := http.Get(u.String())
	if err != nil {
		return "", err
	}
	rv, err := returnValue(resp)
	resp.Body.Close()
	return rv, err
}

func downloadData(u string) (string, error) {
	resp, err := http.Get(u)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()
	tmp, err := ioutil.TempFile("", "wnlf")
	if err != nil {
		return "", err
	}
	defer tmp.Close()
	_, err = io.Copy(tmp, resp.Body)
	if err != nil {
		return "", err
	}
	return tmp.Name(), nil
}

func unzip(zname, base string) error {
	zf, err := os.Open(zname)
	if err != nil {
		return err
	}

	fi, err := os.Stat(zname)
	if err != nil {
		return err
	}

	zr, err := zip.NewReader(zf, fi.Size())
	if err != nil {
		return err
	}
	for _, f := range zr.File {
		p := base + filepath.Ext(f.Name)
		fout, err := os.Create(p)
		if err != nil {
			return err
		}
		rc, err := f.Open()
		if err != nil {
			return err
		}
		_, err = io.Copy(fout, rc)
		if err != nil {
			return err
		}
		rc.Close()
		fout.Close()
	}
	return nil
}

func Get(e Envelope, start, stop time.Time, dst string) error {
	log.SetFlags(log.LstdFlags | log.Lshortfile)
	var u *url.URL
	var err error
	if u, err = requestValidation(e, "F8V30HZ"); err != nil {
		return err
	}

	// Update the target SRS with the proper UTM EPSG code
	q := u.Query()
	q.Set("prj", "32612")
	u.RawQuery = q.Encode()

	resp, err := http.Get(u.String())
	if err != nil {
		return err
	}
	id, err := returnValue(resp)
	resp.Body.Close()

	if err = initiateDownload(id); err != nil {
		return err
	}

	var statusMap = map[int]string{
		100: "received",
		200: "initiated",
		210: "extracting",
		390: "finishing",
		400: "ready",
		900: "extraction error",
	}

	tries := 0
	var status int
	for {
		tries++
		if tries > 30 {
			err = fmt.Errorf("landfire client timed out")
			break
		}
		status, err = downloadStatus(id)
		if err != nil {
			return err
		}
		log.Printf("%d: %s...", status, statusMap[status])
		if status >= 400 {
			break
		}
		time.Sleep(5 * time.Second)
	}
	if status != 400 {
		return fmt.Errorf("failed to download data")
	}

	dlURL, err := getData(id)
	if err != nil {
		return err
	}

	zname, err := downloadData(dlURL)
	if err != nil {
		return err
	}
	base := dst[:len(filepath.Ext(dst))]
	if err = unzip(zname, base); err != nil {
		return err
	}
	return nil
}
