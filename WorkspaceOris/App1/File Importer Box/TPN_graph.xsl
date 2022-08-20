<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xs="http://www.w3.org/2001/XMLSchema">
	<xsl:template match="/">
		<html>
			<head/>
			<body>
				<xsl:for-each select="Grafo">
					<span style="color:blue; font-weight:bold">TPN Graph</span>
					<xsl:if test="position()=1">
						<table border="0">
							<xsl:if test="position()=1">
								<xsl:text disable-output-escaping="yes">&lt;tbody&gt;</xsl:text>
							</xsl:if>
							<tr>
								<td>
									<span style="color:teal; font-style:italic">name:</span>
								</td>
								<xsl:for-each select="../Grafo">
									<td width="164">
										<xsl:for-each select="@name">
											<span style="color:red">
												<span style="color:red">
													<xsl:value-of select="."/>
												</span>
											</span>
										</xsl:for-each>
									</td>
								</xsl:for-each>
							</tr>
							<tr>
								<td>
									<span style="color:teal; font-style:italic">level:</span>
								</td>
								<xsl:for-each select="../Grafo">
									<td width="164">
										<xsl:for-each select="@level">
											<span style="color:red">
												<span style="color:red">
													<xsl:value-of select="."/>
												</span>
											</span>
										</xsl:for-each>
									</td>
								</xsl:for-each>
							</tr>
							<tr>
								<td>
									<span style="color:teal; font-style:italic">state:</span>
								</td>
								<xsl:for-each select="../Grafo">
									<td width="164">
										<xsl:for-each select="@state">
											<span style="color:red">
												<span style="color:red">
													<xsl:value-of select="."/>
												</span>
											</span>
										</xsl:for-each>
									</td>
								</xsl:for-each>
							</tr>
							<xsl:if test="position()=last()">
								<xsl:text disable-output-escaping="yes">&lt;/tbody&gt;</xsl:text>
							</xsl:if>
						</table>
					</xsl:if>
					<xsl:for-each select="class">
						<div>
							<table border="0">
								<xsl:if test="position()=1">
									<xsl:text disable-output-escaping="yes">&lt;tbody&gt;</xsl:text>
								</xsl:if>
								<tr>
									<td>
										<span style="color:purple; font-weight:bold">
											<xsl:element name="a">
													<xsl:attribute name="name">#<xsl:value-of select="@id"/></xsl:attribute>
												Class</xsl:element>
										</span>
									</td>
									<td>
										<span style="color:teal; font-style:italic">level</span>
										<span style="color:teal">:</span>
									</td>
									<td>
										<xsl:for-each select="@level">
											<span style="color:red">
												<span style="color:red">
													<xsl:value-of select="."/>
												</span>
											</span>
										</xsl:for-each>
									</td>
									<td>
										<span style="font-style:italic">, </span>
										<span style="color:teal; font-style:italic">id</span>
										<span style="color:teal">:</span>
									</td>
									<td>
										<xsl:for-each select="@id">
											<span style="color:red">
												<span style="color:red">
													<xsl:value-of select="."/>
												</span>
											</span>
										</xsl:for-each>
									</td>
								</tr>
								<xsl:if test="position()=last()">
									<xsl:text disable-output-escaping="yes">&lt;/tbody&gt;</xsl:text>
								</xsl:if>
							</table>
						</div>
						<xsl:for-each select="place">
							<div>
								<table>
									<xsl:if test="position()=1">
										<xsl:text disable-output-escaping="yes">&lt;tbody&gt;</xsl:text>
									</xsl:if>
									<tr>
										<td width="43"/>
										<td>
											<span style="color:maroon; font-weight:bold">Place</span>
										</td>
										<td>
											<span style="color:teal; font-style:italic">id:</span>
										</td>
										<td>
											<xsl:for-each select="@id">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>

										<td>, <span style="color:teal"/>
											<span style="color:teal; font-style:italic">name:</span>
										</td>
										<td>
											<xsl:for-each select="@name">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>

										<td>, <span style="color:teal"/>
											<span style="color:teal; font-style:italic">tokens:</span>
										</td>
										<td>
											<xsl:for-each select="@tokens">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
									</tr>
									<xsl:if test="position()=last()">
										<xsl:text disable-output-escaping="yes">&lt;/tbody&gt;</xsl:text>
									</xsl:if>
								</table>
							</div>
						</xsl:for-each>

						<xsl:for-each select="transx">
							<div>
								<xsl:apply-templates/>
								<table border="0">
									<xsl:if test="position()=1">
										<xsl:text disable-output-escaping="yes">&lt;tbody&gt;</xsl:text>
									</xsl:if>
									<tr>
										<td width="41"/>
										<td>
											<span style="color:maroon; font-weight:bold">
												<xsl:choose>
													<xsl:when test="count(@class_dest)=1">
														<xsl:element name="a">																											<xsl:attribute name="href">#<xsl:value-of select="@class_dest"/></xsl:attribute>
														Trans</xsl:element>
													</xsl:when>
													<xsl:otherwise>	
														Trans
													</xsl:otherwise>
												</xsl:choose>
											</span>
										</td>
										<td>
											<span style="color:teal; font-style:italic">id:</span>
										</td>
										<td>
											<xsl:for-each select="@id">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
										<td>, <span style="color:teal; font-style:italic">name:</span>
										</td>
										<td>
											<xsl:for-each select="@name">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
										<td>, <span style="color:teal; font-style:italic">progr:</span>
										</td>
										<td>
											<xsl:for-each select="@Progr">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
										<td>, <span style="color:teal; font-style:italic">Newly:</span>
										</td>
										<td>
											<xsl:for-each select="@New">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
										<td>, <span style="color:teal; font-style:italic">EFT:</span>
										</td>
										<td>
											<xsl:for-each select="@EFT">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
										<td>, <span style="color:teal; font-style:italic">LFT:</span>
										</td>
										<td>
											<xsl:for-each select="@LFT">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
										<td>, <span style="color:teal; font-style:italic">Firable:</span>
										</td>
										<td>
											<xsl:for-each select="@Fir">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
										<td>
											<xsl:if test="count (@class_dest)=1">
											, <span style="color:teal; font-style:italic">Dest class:</span>
											</xsl:if>
										</td>
										<td>
											<xsl:for-each select="@class_dest">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
									</tr>
									<xsl:if test="position()=last()">
										<xsl:text disable-output-escaping="yes">&lt;/tbody&gt;</xsl:text>
									</xsl:if>
								</table>
							</div>
						</xsl:for-each>

						<xsl:for-each select="binding">
							<div>
								<xsl:apply-templates/>
								<table border="0">
									<xsl:if test="position()=1">
										<xsl:text disable-output-escaping="yes">&lt;tbody&gt;</xsl:text>
									</xsl:if>
									<tr>
										<td width="41"/>

										<td>
											<span style="color:maroon; font-weight:bold">Binding</span>
										</td>
										<td>
											<span style="color:teal; font-style:italic">idtrans1:</span>
										</td>
										<td>
											<xsl:for-each select="@idtrans1">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
										<td>, <span style="color:teal; font-style:italic">idtrans2:</span>
										</td>
										<td>
											<xsl:for-each select="@idtrans2">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
										<td>, <span style="color:teal; font-style:italic">EFT:</span>
										</td>
										<td>
											<xsl:for-each select="@EFT">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
										<td>, <span style="color:teal; font-style:italic">LFT:</span>
										</td>
										<td>
											<xsl:for-each select="@LFT">
												<span style="color:red">
													<span style="color:red">
														<xsl:value-of select="."/>
													</span>
												</span>
											</xsl:for-each>
										</td>
									</tr>
									<xsl:if test="position()=last()">
										<xsl:text disable-output-escaping="yes">&lt;/tbody&gt;</xsl:text>
									</xsl:if>
								</table>
							</div>
						</xsl:for-each>						
						
					</xsl:for-each>
				</xsl:for-each>
			</body>
		</html>
	</xsl:template>
</xsl:stylesheet>
