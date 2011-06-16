from pyparsing import *
import sys,re

debug = False 

def getMediaWikiFirstLink(text):
	links = parseMediaWiki(text)

	if len(links) == 0:
		raise ParseException('no links found')
	
	links = [ln for ln in links if re.search('^(file|image)',ln.lower()) is None]
	firstlink = links[0]
	if firstlink.find('|') > -1:
		firstlink = firstlink[:firstlink.find('|')]

	return firstlink

def parseMediaWiki(text):

	# ############################# Grammer #################################

	textNoStop = Regex('[^\s\{\}\[\]\(\)]+')
	myHtmlComment = QuotedString("<!--",endQuoteChar="-->",multiline=True)
	regularText = (textNoStop ^ Literal("[") ^ Literal("]") ) 

	regularBrackets = Forward() 
	regularBrackets << Combine(Literal("(") + ZeroOrMore(Regex('[^\(\)]+') ^ regularBrackets) + Literal(")"))

	link = Forward()
	link << Combine( Literal("[[").suppress() + ZeroOrMore(Regex('[^\[\]]+') ^ link) + Literal("]]").suppress()) 

	curlyShit = Forward()
	curlyShit << Combine( Literal("{{") + ZeroOrMore( Regex('[^\{\}]+') ^ curlyShit ) + Literal("}}") , joinString=" ") 

#	curlyCurlyBar = Forward()
#	curlyCurlyBar << Combine( Literal("{|") + ZeroOrMore( Regex('[^\{]+') ^ curlyShit ) + Literal("|}") , joinString=" ") 
	curlyCurlyBar = QuotedString("{|",endQuoteChar="|}",multiline=True)+Optional(QuotedString("}",endQuoteChar="|}",multiline=True))
	strangeCurlyBar = QuotedString("|",endQuoteChar="|}",multiline=True) #+NotAny(Literal("}")) # strangely it may also appear like this...
	curlyBar = curlyCurlyBar ^ strangeCurlyBar

	strangeBeginRemark = Combine(Literal(":") + QuotedString("''") , joinString=" ")

	if debug:
		wikiMarkup = OneOrMore(regularText ^ strangeBeginRemark ^ curlyBar ^ curlyShit ^ myHtmlComment ^ link ^ regularBrackets)
	else:
		wikiMarkup = Optional(OneOrMore(regularText.suppress() ^ strangeBeginRemark.suppress() ^ curlyBar.suppress() ^ curlyShit.suppress() ^ myHtmlComment.suppress() ^ link ^ regularBrackets.suppress()))

	return wikiMarkup.parseString(text)

if __name__=="__main__":
	if len(sys.argv) > 2 and sys.argv[2] == "debug": debug = True
	text = open(sys.argv[1]).read().decode('utf-8').encode('ascii','ignore')
	print "Original\n\n",text
	print "parse\n\n",parseMediaWiki(text),"\n\n"
	print "first link\n\n",getMediaWikiFirstLink(text),"\n\n"
