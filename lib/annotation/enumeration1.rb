
module Annotation
  class Enumeration1 < Enumeration

    def mark_span(first, last)
      # Also mark first item
      super(first.prev, last)
    end

  end
end
