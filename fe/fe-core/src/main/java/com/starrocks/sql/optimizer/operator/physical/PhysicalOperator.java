// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.

package com.starrocks.sql.optimizer.operator.physical;

import com.starrocks.sql.optimizer.base.ColumnRefSet;
import com.starrocks.sql.optimizer.base.DistributionSpec;
import com.starrocks.sql.optimizer.base.OrderSpec;
import com.starrocks.sql.optimizer.operator.Operator;
import com.starrocks.sql.optimizer.operator.OperatorType;
import com.starrocks.sql.optimizer.operator.scalar.ScalarOperator;

public abstract class PhysicalOperator extends Operator {
    protected long limit = -1;
    protected ScalarOperator predicate;

    public long getLimit() {
        return limit;
    }

    public void setLimit(long limit) {
        this.limit = limit;
    }

    public ScalarOperator getPredicate() {
        return predicate;
    }

    public void setPredicate(ScalarOperator predicate) {
        this.predicate = predicate;
    }

    protected OrderSpec orderSpec;
    protected DistributionSpec distributionSpec;

    protected PhysicalOperator(OperatorType type) {
        this(type, DistributionSpec.createAnyDistributionSpec(), OrderSpec.createEmpty());
    }

    protected PhysicalOperator(OperatorType type, DistributionSpec distributionSpec) {
        this(type, distributionSpec, OrderSpec.createEmpty());
    }

    protected PhysicalOperator(OperatorType type, OrderSpec orderSpec) {
        this(type, DistributionSpec.createAnyDistributionSpec(), orderSpec);
    }

    protected PhysicalOperator(OperatorType type, DistributionSpec distributionSpec,
                               OrderSpec orderSpec) {
        super(type);
        this.distributionSpec = distributionSpec;
        this.orderSpec = orderSpec;
    }

    public OrderSpec getOrderSpec() {
        return orderSpec;
    }

    public DistributionSpec getDistributionSpec() {
        return distributionSpec;
    }

    @Override
    public boolean isPhysical() {
        return true;
    }

    public ColumnRefSet getUsedColumns() {
        ColumnRefSet result = new ColumnRefSet();
        if (predicate != null) {
            result.union(predicate.getUsedColumns());
        }

        if (orderSpec != null) {
            orderSpec.getOrderDescs().forEach(o -> result.union(o.getColumnRef()));
        }

        return result;
    }

    public abstract boolean equals(Object o);

    public abstract int hashCode();
}