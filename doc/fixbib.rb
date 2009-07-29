#!/usr/bin/env ruby
bib = File.read('export.bib')
bib.gsub! '{\\textasciitilde}', '~'
bib.gsub! '\\#', '#'
bib.gsub! '#', '\\#'
bib.gsub! /&/, '\\\\&'
File.open('export.bib', 'w') { |f| f << bib }
