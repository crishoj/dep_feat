
require 'lib/conll/sentence'

module Conll
  class Corpus
    attr_reader :sentences

    def self.parse(file)
      corpus = Corpus.new do |corpus|
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

    def initialize
      @sentences = []
      yield self if block_given?
      self
    end
  end
end
