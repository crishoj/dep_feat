
require 'lib/conll/sentence'

module Conll
  class Corpus
    attr_reader :sentences

    def self.parse(file)
      sentences = []
      File.new(file).each("\n\n") do |part|
        sentences << Conll::Sentence.parse(part.split(/\n/))
      end
      Corpus.new(sentences)
    end

    def initialize(sentences)
      @sentences = sentences
    end
  end
end
