
module Annotation
  class Citation < Annotator

    def pre_sentence
      case @sentence.forms.count('"')
      when 2 then
        # Citation enclosed in quotation marks
        first = @sentence.forms.index('"')
        last  = @sentence.forms[(first+1)..-1].index('"')+(first+1)
        mark_span(first, last)
      when 1 then
        index = @sentence.forms.index('"')
        if index > 0 and @sentence.forms[index-1] == ':'
          # Open-ended citation after colon
          mark_span(index, :end)
        end
      end
    end

    private

    def mark_span(first, last)
      last = (@sentence.tokens.size-1) if last == :end
      @sentence.tokens[first..last].each do |token|
        token.features << 'citation'
      end
    end

  end
end