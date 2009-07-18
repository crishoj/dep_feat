# encoding: UTF-8

module Annotation
  class Span < Base

    protected

    def mark_span(first, last)
      @sentence.tokens[first.index .. last.index].each do |token|
        token.features << feature unless token.features.include? feature
      end
    end

  end
end
