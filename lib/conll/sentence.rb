
require 'lib/conll/token'

module Conll

  class TokenList < Array

    def forms
      self.collect { |tok| tok.form }
    end

  end

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
      @tokens = TokenList.new
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

    def to_s
      @tokens.join("\n")
    end

  end
end
