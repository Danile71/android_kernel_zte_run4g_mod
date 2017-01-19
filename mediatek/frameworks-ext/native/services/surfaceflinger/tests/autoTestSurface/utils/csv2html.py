

import sys, os, time
import re

import csv

import signal

from mtk_utils import *

from threading import Thread



def html_header() :
    my_str = '''<html>
  <head>
    <script type="text/javascript" src="https://www.google.com/jsapi"></script>
    <script type="text/javascript">
      google.load("visualization", "1", {packages:["corechart"]});
      google.setOnLoadCallback(drawChart);
      function drawChart() {
        var my_array = ['''
    return my_str


def html_rest(chart_title) :
    my_str_p1 = ''']
            
        var data = google.visualization.arrayToDataTable(my_array);

        var options = {
          title: "'''
          
    my_str_p2 = '''"
        };

        var chart = new google.visualization.LineChart(document.getElementById("chart_div"));
        chart.draw(data, options);
      }
    </script>
  </head>
  <body>
    <div id="chart_div" style="width: 900px; height: 500px;"></div>
  </body>
</html>
'''
    return my_str_p1 + chart_title + my_str_p2




class csv2html (object) :
    def __init__ (self) :
        pass
        
    def write_html(self, csvFile, chartTitle, output_html) :

        _csv_file = csvFile 
        _chart_title = chartTitle        
        
        with open(output_html,'wb') as out_file :
            
            # write html header
            out_file.write(html_header())
        
            with open(_csv_file,'rb') as in_file :
                
                rows = in_file.readlines()
                
                for idx, row in enumerate(rows) :
                    
                    ret = row.replace('\r\n','')
                    
                    my_str = '[%s]' % (ret)
                    
                    #my_str = ', ["%s",%d,%d,%d,%d,%d,%d]' % (logtime, pvr_size, sf_size, ms_size, gallery_size, system_ui_size, launcher_size)
                    if idx > 0 :
                        my_str = ', ' + my_str
                    
                    out_file.write(my_str)
                
                
                
            #with open(self._outFile,'a+b') as outfile:
            out_file.write(html_rest(_chart_title))
        

if __name__ == '__main__' :
    
    t = csv2html()
    
    t.write_html('log_ion_heap.csv', 'ION_HEAP','log_ion_heap.html')
    
