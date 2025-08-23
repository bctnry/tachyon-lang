package main

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
)

type Datum interface {
	Type() int
	String() string
	Clone() Datum
}
type DatumList struct { sealed bool; v []Datum }
type DatumInteger struct { v int }
type DatumWord struct { v string }
type DatumBool struct { v bool }
func dEmptyList() *DatumList { return &DatumList{sealed: false, v: make([]Datum, 0)} }
func dInt(v int) *DatumInteger { return &DatumInteger{v: v} }
func dWord(v string) *DatumWord { return &DatumWord{v: v} }
func dBool(v bool) *DatumBool { return &DatumBool{v: v} }
func readWord(d Datum) string {
	w1, ok := d.(*DatumWord)
	if ok { return w1.v }
	w2, ok := d.(*DatumInteger)
	if ok { return fmt.Sprintf("%d", w2.v) }
	w3, ok := d.(*DatumBool)
	if ok { if w3.v { return "TRUE" } else { return "FALSE" } }
	panic("invalid datum type for read word")
}		

const (
	TYPE_INTEGER = 1
	TYPE_WORD = 2
	TYPE_BOOL = 3
	TYPE_LIST = 4
)
func (l *DatumInteger) Type() int { return TYPE_INTEGER }
func (l *DatumList) Type() int { return TYPE_LIST }
func (l *DatumWord) Type() int { return TYPE_WORD }
func (l *DatumBool) Type() int { return TYPE_BOOL }
func (l *DatumInteger) String() string { return fmt.Sprintf("%d", l.v) }
func (l *DatumBool) String() string { if l.v { return "TRUE" } else { return "FALSE" } }
func (l *DatumWord) String() string { return l.v }
func (l *DatumList) String() string {
	r := make([]string, 0)
	r = append(r, "[")
	for _, k := range l.v {
		r = append(r, k.String())
	}
	r = append(r, "]")
	return strings.Join(r, " ")
}
func (l *DatumInteger) Clone() Datum { return dInt(l.v) }
func (l *DatumBool) Clone() Datum { return dBool(l.v) }
func (l *DatumList) Clone() Datum { return &DatumList{sealed: l.sealed, v: l.v} }
func (l *DatumWord) Clone() Datum { return dWord(l.v) }


const (
	STATE_NORMAL = 1
	STATE_ESCAPE = 2
)

type Env struct {
	state int
	escapeCount int
	envPage []map[string]Datum
	stack []Datum
}

func New() *Env {
	env := make([]map[string]Datum, 0)
	env = append(env, make(map[string]Datum, 0))
	return &Env{
		state: STATE_NORMAL,
		escapeCount: 0,
		envPage: env,
		stack: make([]Datum, 0),
	}
}

func isInt(s string) bool {
	for _, k := range s {
		if ! ('0' <= k && k <= '9') { return false }
	}
	return true
}

func (e *Env) enter() {
	e.envPage = append(e.envPage, make(map[string]Datum, 0))
}

func (e *Env) leave() {
	e.envPage = e.envPage[:len(e.envPage)-1]
}

func (e *Env) lookup(n string) Datum {
	i := len(e.envPage)-1
	for i >= 0 {
		p := e.envPage[i]
		k, ok := p[n]
		if !ok { i -= 1; continue }
		return k
	}
	return nil
}

func dumpStack(s []Datum) string {
	r := make([]string, 0)
	i := len(s) - 1
	for i >= 0 {
		r = append(r, s[i].String())
		i -= 1
	}
	return strings.Join(r, "\n")
}

func (e *Env) insert(n string, v Datum) {
	e.envPage[len(e.envPage)-1][n] = v
}

func (e *Env) exec(r Datum) {
	if e.state == STATE_ESCAPE {
		switch r.Type() {
		case TYPE_WORD:
			k := readWord(r)
			switch k {
			case "]":
				var l *DatumList
				e.stack[len(e.stack)-1].(*DatumList).sealed = true
				if len(e.stack) >= 2 {
					ll, ok := e.stack[len(e.stack)-2].(*DatumList)
					if ok && !ll.sealed {
						e.stack, l = e.stack[:len(e.stack)-1], e.stack[len(e.stack)-1].(*DatumList)
						l.sealed = true
						ll.v = append(ll.v, l)
					}
				}
				e.escapeCount -= 1
				if e.escapeCount <= 0 {
					e.state = STATE_NORMAL
				}
			case "[":
				e.stack = append(e.stack, dEmptyList())
				e.escapeCount += 1
			default:
				l := e.stack[len(e.stack)-1].(*DatumList)
				if isInt(k) {
					kk, _ := strconv.Atoi(k)
					l.v = append(l.v, dInt(kk))
				} else {
					l.v = append(l.v, dWord(k))
				}
			}
		default:
			l := e.stack[len(e.stack)-1].(*DatumList)
			l.v = append(l.v, r)
		}
		return
	}
	switch r.Type() {
	case TYPE_WORD:
		k := readWord(r)
		switch k {
		case "[":
			e.stack = append(e.stack, dEmptyList())
			e.state = STATE_ESCAPE
			e.escapeCount += 1
		case "]":
			panic("not in escape mode")
		case "$":
			var w Datum
			e.stack, w = e.stack[:len(e.stack)-1], e.stack[len(e.stack)-1]
			name := readWord(w)
			e.stack = append(e.stack, e.lookup(name))
		case "#":
			var name string
			var value Datum
			e.stack, name, value = e.stack[:len(e.stack)-2], readWord(e.stack[len(e.stack)-2]), e.stack[len(e.stack)-1]
			e.insert(name, value)
		case "begin":
			e.enter()
		case "end":
			e.leave()
		case "TRUE":
			e.stack = append(e.stack, dBool(true))
		case "FALSE":
			e.stack = append(e.stack, dBool(false))
		default:
			if len(k) <= 0 { panic("cannot have empty word") }
			if k[0] == '\'' {
				e.stack = append(e.stack, dWord(k[1:]))
			} else {
				switch k {
				case "drop":
					e.stack = e.stack[:len(e.stack)-1]
				case "swap":
					a := e.stack[len(e.stack)-1]
					b := e.stack[len(e.stack)-2]
					e.stack = e.stack[:len(e.stack)-2]
					e.stack = append(e.stack, a)
					e.stack = append(e.stack, b)
				case "dup":
					e.stack = append(e.stack, e.stack[len(e.stack)-1].Clone())
				case "over":
					e.stack = append(e.stack, e.stack[len(e.stack)-2].Clone())
				case "rot3":
					l := e.stack[:len(e.stack)-3]
					l = append(l, e.stack[len(e.stack)-2])
					l = append(l, e.stack[len(e.stack)-1])
					l = append(l, e.stack[len(e.stack)-3])
					e.stack = l
				case "inc":
					w := e.stack[len(e.stack)-1]
					w.(*DatumInteger).v += 1
				case "dec":
					w := e.stack[len(e.stack)-1]
					w.(*DatumInteger).v -= 1
				case "+":
					var w1, w2 *DatumInteger
					e.stack, w1, w2 = e.stack[:len(e.stack)-2], e.stack[len(e.stack)-2].(*DatumInteger), e.stack[len(e.stack)-1].(*DatumInteger)
					e.stack = append(e.stack, dInt(w1.v+w2.v))
				case "*":
					var w1, w2 *DatumInteger
					e.stack, w1, w2 = e.stack[:len(e.stack)-2], e.stack[len(e.stack)-2].(*DatumInteger), e.stack[len(e.stack)-1].(*DatumInteger)
					e.stack = append(e.stack, dInt(w1.v*w2.v))
				case "<=":
					var w1, w2 *DatumInteger
					e.stack, w1, w2 = e.stack[:len(e.stack)-2], e.stack[len(e.stack)-2].(*DatumInteger), e.stack[len(e.stack)-1].(*DatumInteger)
					e.stack = append(e.stack, dBool(w1.v <= w2.v))
				case "==":
					var w1, w2 *DatumInteger
					e.stack, w1, w2 = e.stack[:len(e.stack)-2], e.stack[len(e.stack)-2].(*DatumInteger), e.stack[len(e.stack)-1].(*DatumInteger)
					e.stack = append(e.stack, dBool(w1.v == w2.v))
				case "if":
					var c *DatumBool
					var t, ec Datum
					e.stack, c, t, ec = e.stack[:len(e.stack)-3], e.stack[len(e.stack)-3].(*DatumBool), e.stack[len(e.stack)-2], e.stack[len(e.stack)-1]
					if c.v {
						e.execList(t.(*DatumList).v)
					} else {
						e.execList(ec.(*DatumList).v)
					}
				case "dstk":
					fmt.Printf("%s\n", dumpStack(e.stack))
				default:
					v := e.lookup(k)
					if v == nil { panic(fmt.Sprintf("not found: %s", k)) }
					e.execList(v.(*DatumList).v)
				}
			}
		}
	default:
		e.stack = append(e.stack, r)
	}
}

func (e *Env) execList(l []Datum) {
	for _, k := range l { e.exec(k) }
}

func isWhitespace(s rune) bool {
	return strings.Contains(" \t\r\n\b\v\f", string(s))
}

type Token struct { ttype int; v string }
const (
	TOKEN_WHITESPACE = 1
	TOKEN_NEWLINE = 2
	TOKEN_INTEGER = 3
	TOKEN_WORD = 4
)

func readTokenFromLine(s string) []Datum {
	r := make([]Datum, 0)
	rs := []rune(s)
	i := 0
	for i < len(rs) {
		k := rs[i]
		if k == '[' { r = append(r, dWord("[")); i += 1; continue }
		if k == ']' { r = append(r, dWord("]")); i += 1; continue }
		if k == '#' { r = append(r, dWord("#")); i += 1; continue }
		if k == '$' { r = append(r, dWord("$")); i += 1; continue }
		if k == '/' {
			if i+1 < len(rs) && rs[i+1] == '/' {
				break
			}
		}
		if isWhitespace(k) {
			for isWhitespace(k) && i < len(rs) {
				i += 1
				if i >= len(rs) { break }
				k = rs[i]
			}
			continue
		}
		s := make([]rune, 0)
		for i < len(rs) && k != '[' && k != ']' && k != '#' && k != '$' && !isWhitespace(k) {
			s = append(s, k)
			i += 1
			if i >= len(rs) { break }
			k = rs[i]
		}
		ss := string(s)
		if isInt(ss) {
			a, _ := strconv.Atoi(ss)
			r = append(r, dInt(a))
		} else {
			r = append(r, dWord(ss))
		}
	}
	return r
}

func main() {
	e := New()
	br := bufio.NewReader(os.Stdin)
	for {
		fmt.Printf(">>> ")
		for {
			t, err := br.ReadString('\n')
			if errors.Is(err, io.EOF) {
				goto leave;
			}
			if err != nil { panic(err) }
			dl := readTokenFromLine(t)
			for _, k := range dl {
				e.exec(k)
			}
			if (e.state == STATE_NORMAL) { break }
			fmt.Printf("  | ")
		}
	}
leave:
}

