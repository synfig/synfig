/**
 * SynfigTween — Lightweight tween engine for PixiJS
 * Supports: linear, ease-in, ease-out, ease-in-out, cubic-bezier, constant
 */
class SynfigTween {
  constructor(target) {
    this.target = target;
    this.keyframes = [];
    this.duration = 0;
    this.loop = false;
    this.playing = false;
    this._startTime = 0;
  }

  addKeyframe(time, props, easing = 'linear') {
    this.keyframes.push({ time, props: { ...props }, easing });
    this.keyframes.sort((a, b) => a.time - b.time);
    this.duration = Math.max(this.duration, time);
    return this;
  }

  play(ticker) {
    this.playing = true;
    this._startTime = performance.now();
    this._tickerRef = () => this._update();
    this._ticker = ticker;
    ticker.add(this._tickerRef);
    return this;
  }

  stop() {
    this.playing = false;
    if (this._ticker && this._tickerRef) {
      this._ticker.remove(this._tickerRef);
    }
  }

  _update() {
    if (!this.playing) return;
    let elapsed = (performance.now() - this._startTime) / 1000;
    if (this.loop && this.duration > 0) {
      elapsed = elapsed % this.duration;
    } else if (elapsed > this.duration) {
      elapsed = this.duration;
      this.stop();
    }
    this._applyAt(elapsed);
  }

  _applyAt(t) {
    if (this.keyframes.length === 0) return;
    if (t <= this.keyframes[0].time) {
      for (const key of Object.keys(this.keyframes[0].props)) {
        const val = this.keyframes[0].props[key];
        if (typeof val === 'number') this.target[key] = val;
      }
      return;
    }
    let prev = this.keyframes[0];
    let next = this.keyframes[this.keyframes.length - 1];
    for (let i = 0; i < this.keyframes.length - 1; i++) {
      if (t >= this.keyframes[i].time && t <= this.keyframes[i + 1].time) {
        prev = this.keyframes[i];
        next = this.keyframes[i + 1];
        break;
      }
    }
    const segDuration = next.time - prev.time;
    const progress = segDuration > 0 ? (t - prev.time) / segDuration : 1;
    const easedProgress = SynfigTween.ease(progress, next.easing);

    for (const key of Object.keys(next.props)) {
      const from = prev.props[key] !== undefined ? prev.props[key] : this.target[key];
      const to = next.props[key];
      if (typeof from === 'number' && typeof to === 'number') {
        this.target[key] = from + (to - from) * easedProgress;
      }
    }
  }

  static ease(t, type) {
    switch (type) {
      case 'constant': return 0;
      case 'linear': return t;
      case 'ease-in': return t * t;
      case 'ease-out': return t * (2 - t);
      case 'ease-in-out': return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
      default:
        if (Array.isArray(type) && type.length === 4) {
          return SynfigTween.cubicBezier(type[0], type[1], type[2], type[3], t);
        }
        return t;
    }
  }

  static cubicBezier(x1, y1, x2, y2, t) {
    const cx = 3 * x1, bx = 3 * (x2 - x1) - cx, ax = 1 - cx - bx;
    let x = t;
    for (let i = 0; i < 8; i++) {
      const curveX = ((ax * x + bx) * x + cx) * x;
      const curveXDeriv = (3 * ax * x + 2 * bx) * x + cx;
      if (Math.abs(curveX - t) < 1e-6) break;
      if (Math.abs(curveXDeriv) < 1e-6) break;
      x -= (curveX - t) / curveXDeriv;
      x = Math.max(0, Math.min(1, x));
    }
    const cy = 3 * y1, by = 3 * (y2 - y1) - cy, ay = 1 - cy - by;
    return ((ay * x + by) * x + cy) * x;
  }
}
