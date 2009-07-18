
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

    def last?
      @index == (@corpus.sentences.size - 1)
    end

    def to_s
      @tokens.join("\n")
    end

    def grep(options = {})
      for token in tokens
        next if options[:pos]     and not token.pos == options[:pos]
        next if options[:cpos]    and not token.cpos == options[:cpos]
        next if options[:form]    and not token.form == options[:form]
        next if options[:feat]    and not token.features.include? options[:feat]
        next if options[:deprel]  and not token.deprel == options[:deprel]
        next if options[:form_re] and not token.form =~ options[:form_re]
        # Matched
        yield token
      end
    end

  end
end
