
require 'lib/conll/token'

module Conll
  class Sentence
    attr_accessor :corpus, :index
    attr_reader :tokens, :root

    def self.parse(lines)
      Sentence.new do |sentence|
        lines.each do |line|
          sentence << Token.parse(line)
        end
      end
    end

    def initialize
      @root   = RootToken.new
      @root.sentence = self
      @tokens = []
      yield self if block_given?
      self
    end

    def <<(token)
      token.sentence = self
      @tokens << token
    end

    def next
      @corpus.sentences[@index.succ]
    end

    def forms
      @tokens.collect { |tok| tok.form }
    end

    def last?
      @index == (@corpus.sentences.size - 1)
    end

  end
end
