
require 'lib/conll/sentence'

module Conll
  class Corpus
    attr_reader :sentences, :filename

    def self.parse(file)
      corpus = Corpus.new(file) do |corpus|
        File.new(file).each("\n\n") do |part|
          lines = part.split(/\n/)
          corpus << Conll::Sentence.parse(lines)
        end
      end
    end

    def <<(sentence)
      sentence.corpus = self
      sentence.index = @sentences.size
      @sentences << sentence
    end

    def initialize(filename = '')
      @filename  = filename
      @sentences = []
      yield self if block_given?
      self
    end

    def to_s
      @sentences.join("\n\n") + "\n"
    end
    
  end
end
