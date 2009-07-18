
module Annotation
  class Enumeration < Span

    def feature
      'enum'
    end

    def pre_sentence
      @sentence.grep(:cpos => 'C') do |tok|
        if tok.next.cpos == 'N'
          if leading_comma(tok)
            # Seems to be an enumeration
            first = first_comma(tok)
            mark_span(first, tok.next)
          end
        end
      end
    end

    protected

    def leading_comma(token)
      token.leading(3).find { |t| t.form == ',' }
    end

    def first_comma(token)
      while (token = leading_comma(token))
        comma = token
      end
      comma
    end

  end
end
