require 'lib/annotation/span_annotator'

module Annotation
  class Parenthesis < SpanAnnotator

    def feature
      'paren'
    end

    def pre_sentence
      @in_parenthesis = false
    end

    def mark_token
      if @token.form == '('
        @in_parenthesis = true
      elsif @token.form == ')'
        @in_parenthesis = false
      else
        @token.features << feature if @in_parenthesis
      end
    end

  end
end
