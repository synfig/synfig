on open names
	set fileNames to {}
	repeat with i from 1 to count of names
		set fileNames to fileNames & {quoted form of (POSIX path of (item i of names))}
	end repeat
	
	set fifoLocation to " >> ~/sinfg/fifo"
	
	repeat with fileName in fileNames
		set cmdLine to {"echo O " & fileName & fifoLocation}
		do shell script cmdLine
	end repeat
end open

