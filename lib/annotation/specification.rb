
require 'lib/annotation/span'

module Annotation
  class Specification < Span

    MARKERS = %w{, -}

    def feature
      'spec'
    end

    def mark_token
      for marker in MARKERS
        if @token.form == marker
          if @specifier = @token.next
            if @specifier.pos =~ /^N/i and @end = @specifier.next
              if @end.form == marker
                mark_span(@token, @end)
              end
            end
          end
        end
      end
    end

  end
end
