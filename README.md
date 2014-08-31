RC_Practice_Desktop_App
=======================

This is a desktop application for practicing reading comprehensions. Unlike its <a href='https://github.com/lagnajeet/RC_Practice_Utility'>predecessor</a> which was just an HTML file with the logic implemented in JavaScript this is a true desktop application. It accepts data files as XML documents. The data format of the XML document is as shown below.

<pre>
<passages>	
	<passage_data pid="[passage_index]">
		<data txt="[passage_data]"></data>
		<questions>
			<question_data qid="[question_index]">
				<data txt="[question]" index="0"></data>
				<data txt="[answer]" index="1"></data>
				<data txt="[option1]" index="2"></data>
				<data txt="[option2]" index="3"></data>
				<data txt="[option3]" index="4"></data>
				<data txt="[option4]" index="5"></data>
			</question_data>
		</questions>
	</passage_data>
</passages>	
</pre>

The executables can be downloaded from <a href="https://github.com/lagnajeet/RC_Practice_Desktop_App/blob/master/RC_Practice_Utility_32bit_Windows_Linux.zip?raw=true">RC_Practice_Utility_32bit_Windows_Linux</a>. It contains an example <a href="https://github.com/lagnajeet/RC_Practice_Desktop_App/blob/master/data.xml">data.xml</a> file and the program compiled for both Windows and Linux. It can also be compiled for Mac.
<br>
It uses <a href="www.fltk.org">FLTK</a> for the GUI and <a href="rapidxml.sourceforge.net">RapidXML</a> for XML parsing.
