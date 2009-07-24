
module Annotation
  class EPosEnumeration < Enumeration

    def mark_span(first, last)
      super(first, last)
      # Make POS tags on the commas in enumerations unique
      @sentence.tokens[first.index .. last.index].each do |tok|
        if tok.form == ','
          tok.cpos = tok.cpos + '-e'
          tok.pos  = tok.pos + '-e'
        end
      end
    end

  end
end
