
require 'lib/annotation/span'

module Annotation
  class QuotedPhrase < Span
    
    def feature
      'quote'
    end

    def pre_sentence
      case @sentence.forms.count('"')
      when 2 then
        # Word span enclosed in quotation marks
        first = @sentence.forms.index('"') + 1
        last  = @sentence.forms[first..-1].index('"') + first - 1
        mark_span(first, last)
      when 1 then
        index = @sentence.forms.index('"')
        if index > 0 and @sentence.forms[index-1] == ':'
          # Open-ended quotation after colon
          mark_span(index, :end)
        end
      end
    end

  end
end